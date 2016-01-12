#include "MPU6050.h"
#include "stdio.h"
#include "stm32f4xx.h"
#include "I2C.h"
#include "USART.h"
#include "math.h"

// 弧度制转角度制因子
#define ARC_TO_DEG 57.295780f

// 九轴传感器数值
float init_ax, init_ay, init_az,
			init_gx, init_gy, init_gz,
			init_mx, init_my, init_mz;

// 角速度零漂偏移量
float Gyro_Xout_Offset, Gyro_Yout_Offset, Gyro_Zout_Offset;
// 加速度零漂偏移量
float Accel_Xout_Offset, Accel_Yout_Offset, Accel_Zout_Offset;

// 获取角速度零漂偏移量
void Get_Gyro_Bias(void)
{
	uint16_t i;
	int16_t gyro[3];
	int32_t gyro_x = 0, gyro_y = 0, gyro_z = 0;
	static int16_t count = 0;
	uint8_t data_write[6];

	// 取2000次数据
	for(i = 0; i < 2000; i++)
	{
		if(!i2cread(MPU6050_Addr2, GYRO_XOUT_H, 6, data_write))
		{
			// 获取三轴数据
			gyro[0] = ((((int16_t)data_write[0])<<8) | data_write[1]);
			gyro[1] = ((((int16_t)data_write[2])<<8) | data_write[3]);
			gyro[2] = ((((int16_t)data_write[4])<<8) | data_write[5]);
			gyro_x += gyro[0];
			gyro_y += gyro[1];
			gyro_z += gyro[2];
			// 记录有效次数
			count++;
		}
	}
	
	// 求平均值得到零漂偏移量
	Gyro_Xout_Offset = (float)gyro_x / count;
	Gyro_Yout_Offset = (float)gyro_y / count;
	Gyro_Zout_Offset = (float)gyro_z / count;
}

// 获取加速度零漂偏移量
void Get_Accel_Bias(void)
{
	uint32_t i;
	int16_t accel[3]; 
	uint8_t data_write[6];
	float accel_x = 0, accel_y = 0, accel_z = 0;
	static int16_t count2 = 0;
	
	// 取2000次数据
	for(i = 0; i < 2000; i++)
	{
		if(!i2cread(MPU6050_Addr2, ACCEL_XOUT_H, 14, data_write))
		{
			// 获取三轴数据
			accel[0] = (((int16_t)data_write[0])<<8) | data_write[1];
			accel[1] = (((int16_t)data_write[2])<<8) | data_write[3];
			accel[2] = (((int16_t)data_write[4])<<8) | data_write[5];
			accel_x += accel[0];
			accel_y += accel[1];
			accel_z += accel[2];
			// 记录有效次数
			count2++;
		}
	}
	
	// 求平均值得到零漂偏移量
	Accel_Xout_Offset = (float)accel_x / count2;
	Accel_Yout_Offset = (float)accel_y / count2;
	Accel_Zout_Offset = (float)accel_z / count2;
}

// 加速度计与陀螺仪初始化
void MPU6050_Init(void)
{
	// 解除休眠状态
	I2C_WriteByte(MPU6050_Addr, PWR_MGMT_1, 0x01);
	
	// 设置采样频率为1KHZ
	I2C_WriteByte(MPU6050_Addr, SMPLRT_DIV, 0x00);
	// 设置低通滤波的带宽为5HZ
	I2C_WriteByte(MPU6050_Addr, CONFIG, 0x06);
	
	// 开启旁路I2C
	I2C_WriteByte(MPU6050_Addr, INT_PIN_CFG, 0x42);
	// 打开FIFO操作
	I2C_WriteByte(MPU6050_Addr, USER_CTRL, 0x40);
	
	// 设置陀螺仪采集范围为+-500°/s
	I2C_WriteByte(MPU6050_Addr, GYRO_CONFIG, 0x0B);
	// 设置加速度计采集范围为+-4g
	I2C_WriteByte(MPU6050_Addr, ACCEL_CONFIG, 0x08);

	// 获取零漂偏移量
	Get_Gyro_Bias();
	Get_Accel_Bias();
}

// 磁力计初始化
void HMC5883L_Init(void)
{
	// 设置标准数据输出速率为75HZ
	I2C_WriteByte(HMC5883L_Addr, HMC5883L_ConfigurationRegisterA, 0x18);
	// 设置采样频率为+-1.3Ga
	I2C_WriteByte(HMC5883L_Addr, HMC5883L_ConfigurationRegisterB, 0x20);
	// 开启连续测量模式
	I2C_WriteByte(HMC5883L_Addr, HMC5883L_ModeRegister, 0x00);
}

// 采集传感器数据
void Get_AHRS_Data(void)
{
	int16_t gyro[3], accel[3]; 
	uint8_t data_write[14];

	// 获取加速度和角速度
	if(!i2cread(MPU6050_Addr2, ACCEL_XOUT_H, 14, data_write))
	{
		accel[0] = (((int16_t)data_write[0])<<8) | data_write[1];
		accel[1] = (((int16_t)data_write[2])<<8) | data_write[3];
		accel[2] = (((int16_t)data_write[4])<<8) | data_write[5];
		gyro[0]  = (((int16_t)data_write[8])<<8) | data_write[9];
		gyro[1]  = (((int16_t)data_write[10])<<8) | data_write[11];
		gyro[2]  = (((int16_t)data_write[12])<<8) | data_write[13];
		
		// 零漂处理及单位转换
		init_ax = (float)(accel[0] - Accel_Xout_Offset) / Accel_4_Scale_Factor;
		init_ay = (float)(accel[1] - Accel_Yout_Offset) / Accel_4_Scale_Factor;
		init_az = (float)(accel[2] + (Accel_4_Scale_Factor - Accel_Zout_Offset)) / Accel_4_Scale_Factor;
		init_gx = ((float)gyro[0] - Gyro_Xout_Offset) / Gyro_500_Scale_Factor / ARC_TO_DEG;
		init_gy = ((float)gyro[1] - Gyro_Yout_Offset) / Gyro_500_Scale_Factor / ARC_TO_DEG;
		init_gz = ((float)gyro[2] - Gyro_Zout_Offset) / Gyro_500_Scale_Factor / ARC_TO_DEG;
	}
	// 获取磁通量
	if(!i2cread(HMC5883L_Addr2, HMC5883L_XOUT_MSB, 6, data_write))
	{	
		init_mx = (data_write[0] << 8) | data_write[1];
		init_my = (data_write[4] << 8) | data_write[5];
		init_mz = (data_write[2] << 8) | data_write[3];

		// 补码处理及单位转换
		if(init_mx > 0x7fff) init_mx-=0xffff;
		if(init_my > 0x7fff) init_my-=0xffff;
		if(init_mz > 0x7fff) init_mz-=0xffff;
		init_mx /= 1090.0f;
		init_my /= 1090.0f;
		init_mz /= 1090.0f;
	}
}
