#include "AHRS.h"
#include "stdio.h"
#include "math.h"
#include "stm32f4xx.h"
#include "I2C.h"
#include "USART.h"
#include "delay.h"
#include "TIM.h"
#include "MPU6050.h"

// 弧度制转角度制因子
#define ARC_TO_DEG 57.295780f

// proportional gain governs rate of convergence to accelerometer/magnetometer
#define Kp 2.0f
// integral gain governs rate of convergence of gyroscope biases
#define Ki 0.005f

// 四元数
float q0, q1, q2, q3;
// 积分修正值
float exInt = 0, eyInt = 0, ezInt = 0;
// 欧拉角
float Pitch, Roll, Yaw;
// AHRS更新时间的一半
float halfT;
// 偏航初始朝向
float heading;

// 表达式(1.0 / sqrt(value))快速计算算法（快4倍），由美国人John Carmack编写
float invSqrt(float x) 
{
    float halfx = 0.5f * x;
    float y = x;
    long i = *(long*)&y;
    i = 0x5f3759df - (i>>1);
    y = *(float*)&i;
    y = y * (1.5f - (halfx * y * y));
    return y;
}

// 四元数初始化
void Quat_Init(void)
{
    int16_t accel[3];
    uint8_t data_write[6];
    
    // 初始状态下欧拉角数值
    float init_Yaw, init_Pitch, init_Roll;

    // 获取传感器数据
    if(!i2cread(MPU6050_Addr, ACCEL_XOUT_H, 6, data_write))
    {
        accel[0] = ((((int16_t)data_write[0])<<8) | data_write[1]);
        accel[1] = ((((int16_t)data_write[2])<<8) | data_write[3]);
        accel[2] = ((((int16_t)data_write[4])<<8) | data_write[5]);

        // 获得加速度数据
        init_ax = (float)(accel[0] - Accel_Xout_Offset) / Accel_4_Scale_Factor;
        init_ay = (float)(accel[1] - Accel_Yout_Offset) / Accel_4_Scale_Factor;
        init_az = (float)(accel[2] + (Accel_4_Scale_Factor - Accel_Zout_Offset)) / Accel_4_Scale_Factor;
        
        if(!i2cread(HMC5883L_Addr2, HMC5883L_XOUT_MSB, 6, data_write))
        {
            init_mx = (data_write[0] << 8) | data_write[1];
            init_my = (data_write[4] << 8) | data_write[5];
            init_mz = (data_write[2] << 8) | data_write[3];

            if(init_mx > 0x7fff) init_mx-=0xffff;
            if(init_my > 0x7fff) init_my-=0xffff;
            if(init_mz > 0x7fff) init_mz-=0xffff;
            
            // 获得磁通量数据
            init_mx /= 1090.0f;
            init_my /= 1090.0f;
            init_mz /= 1090.0f;
            
            // 机体Y轴向前情况下欧拉角初始值
            init_Roll  = 0.0;
            init_Pitch = 0.0;
            init_Yaw   = atan2(init_mx*cos(init_Roll) + init_my*sin(init_Roll)*sin(init_Pitch) + init_mz*sin(init_Roll)*cos(init_Pitch),
                               init_my*cos(init_Pitch) - init_mz*sin(init_Pitch));

            // 四元数计算
            q0 =  cos(0.5*init_Roll)*cos(0.5*init_Pitch)*cos(0.5*init_Yaw) + sin(0.5*init_Roll)*sin(0.5*init_Pitch)*sin(0.5*init_Yaw);  //w
            q1 =  cos(0.5*init_Roll)*sin(0.5*init_Pitch)*cos(0.5*init_Yaw) - sin(0.5*init_Roll)*cos(0.5*init_Pitch)*sin(0.5*init_Yaw);  //x Pitch
            q2 =  sin(0.5*init_Roll)*cos(0.5*init_Pitch)*cos(0.5*init_Yaw) + cos(0.5*init_Roll)*sin(0.5*init_Pitch)*sin(0.5*init_Yaw);  //y Roll
            q3 =  cos(0.5*init_Roll)*cos(0.5*init_Pitch)*sin(0.5*init_Yaw) - sin(0.5*init_Roll)*sin(0.5*init_Pitch)*cos(0.5*init_Yaw);  //z Yaw

            // 欧拉角弧度制转角度制
            init_Roll  = init_Roll * ARC_TO_DEG;
            init_Pitch = init_Pitch * ARC_TO_DEG;
            init_Yaw   = init_Yaw * ARC_TO_DEG;

            // 获取初始偏航朝向
            heading    = init_Yaw;
        }
    }
}

// AHRS更新算法，由德国人Mahony编写
void AHRS_Update(float gx, float gy, float gz, float ax, float ay, float az, float mx, float my, float mz) 
{
    float norm;
    float hx, hy, hz, bz, by;
    float vx, vy, vz, wx, wy, wz;
    float ex, ey, ez;
    float q0_last, q1_last, q2_last;

    //auxiliary variables to reduce number of repeated operations
    float q0q0 = q0*q0;
    float q0q1 = q0*q1;
    float q0q2 = q0*q2;
    float q0q3 = q0*q3;
    float q1q1 = q1*q1;
    float q1q2 = q1*q2;
    float q1q3 = q1*q3;
    float q2q2 = q2*q2;
    float q2q3 = q2*q3;
    float q3q3 = q3*q3;

    //normalise the measurements
    norm = invSqrt(ax*ax + ay*ay + az*az);
    ax = ax * norm;
    ay = ay * norm;
    az = az * norm;
    norm = invSqrt(mx*mx + my*my + mz*mz);
    mx = mx * norm;
    my = my * norm;
    mz = mz * norm;

    //compute reference direction of flux
    hx = 2*mx*(0.5 - q2q2 - q3q3) + 2*my*(q1q2 - q0q3) + 2*mz*(q1q3 + q0q2);
    hy = 2*mx*(q1q2 + q0q3) + 2*my*(0.5 - q1q1 - q3q3) + 2*mz*(q2q3 - q0q1);
    hz = 2*mx*(q1q3 - q0q2) + 2*my*(q2q3 + q0q1) + 2*mz*(0.5 - q1q1 - q2q2);

    // bx = sqrtf((hx*hx) + (hy*hy));
    by = sqrtf((hx*hx) + (hy*hy));
    bz = hz;

    // estimated direction of gravity and flux (v and w)
    vx = 2*(q1q3 - q0q2);
    vy = 2*(q0q1 + q2q3);
    vz = q0q0 - q1q1 - q2q2 + q3q3;

    wx = 2*by*(q1q2 + q0q3) + 2*bz*(q1q3 - q0q2);
    wy = 2*by*(0.5 - q1q1 - q3q3) + 2*bz*(q0q1 + q2q3);
    wz = 2*by*(q2q3 - q0q1) + 2*bz*(0.5 - q1q1 - q2q2);

    // error is sum of cross product between reference direction of fields and direction measured by sensors
    ex = (ay*vz - az*vy) + (my*wz - mz*wy);
    ey = (az*vx - ax*vz) + (mz*wx - mx*wz);
    ez = (ax*vy - ay*vx) + (mx*wy - my*wx);

    halfT = Get_AHRS_Time();

    if(ex != 0.0f && ey != 0.0f && ez != 0.0f)
    {
        // integral error scaled integral gain
        exInt = exInt + ex*Ki * halfT;
        eyInt = eyInt + ey*Ki * halfT;
        ezInt = ezInt + ez*Ki * halfT;

        // adjusted gyroscope measurements
        gx = gx + Kp*ex + exInt;
        gy = gy + Kp*ey + eyInt;
        gz = gz + Kp*ez + ezInt;
    }

    // save quaternion
    q0_last = q0;
    q1_last = q1;
    q2_last = q2;
    
    // integrate quaternion rate and normalise (Picard first order)
    q0 = q0_last + (-q1_last*gx - q2_last*gy - q3*gz) * halfT;
    q1 = q1_last + ( q0_last*gx + q2_last*gz - q3*gy) * halfT;
    q2 = q2_last + ( q0_last*gy - q1_last*gz + q3*gx) * halfT;
    q3 = q3 + ( q0_last*gz + q1_last*gy - q2_last*gx) * halfT;

    // normalise quaternion
    norm = invSqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
    q0 = q0 * norm;    //w
    q1 = q1 * norm;    //x
    q2 = q2 * norm;    //y
    q3 = q3 * norm;    //z

    // 四元数转欧拉角
    Roll  =  -asin(2*q0*q2 - 2*q1*q3) * ARC_TO_DEG;
    Pitch =  atan2(2*q0*q1 + 2*q2*q3, 1 - 2*q1*q1 - 2*q2*q2) * ARC_TO_DEG;
    Yaw   =  atan2(2*q1*q2 + 2*q0*q3, 1 - 2*q2*q2 - 2*q3*q3) * ARC_TO_DEG;

    // 匿名四轴上位机图像化监控姿态变化
    // usart_report_imu(0, 0, 0, 0, 0, 0, 0, 0, 0, (int)(Roll * 100), (int)(Pitch * 100), (int)(Yaw * 10));
}
