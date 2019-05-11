#include "delay.h"
#include "math.h"
#include "PID.h"
#include "AHRS.h"
#include "TIM.h"
#include "USART.h"
#include "MPU6050.h"
#include "COM.h"

// 积分限幅大小
#define INTEGRAL_MAX     200.0f
#define INTEGRAL_MIN     -200.0f

// PID输出限幅大小
#define PID_OUTPUT_MAX   800.0f
#define PID_OUTPUT_MIN   -800.0f

// PWM输出限幅大小
#define PWM_OUT_MIN      875
#define PWM_OUT_MAX      2000

// PID更新时间
float Time_dt;
// 欧拉角PID输出值
float PID_Roll, PID_Pitch, PID_Yaw;
// 电机转速
uint16_t Motor_1, Motor_2, Motor_3, Motor_4;

// 遥控值
float Motor_Ail = 0.0;
float Motor_Ele = 0.0;
float Motor_Thr = 0.0;
float Motor_Rud = 0.0;
// 遥控中值
float Ail_Mid   = 1527.0;
float Ele_Mid   = 1522.0;
float Rud_Mid   = 1528.0;

// Roll的PID参数
float Roll_Kp        = 1.9;
float Roll_Rate_Kp   = 0.70;
float Roll_Rate_Ti   = 0.10;
float Roll_Rate_Td   = 0.01;
// Pitch的PID参数
float Pitch_Kp       = 2.4;
float Pitch_Rate_Kp  = 0.70;
float Pitch_Rate_Ti  = 0.10;
float Pitch_Rate_Td  = 0.01;
// Yaw的PID参数
float Yaw_Kp         = 0.0;
float Yaw_Rate_Kp    = 0.70;
float Yaw_Rate_Ti    = 0.10;
float Yaw_Rate_Td    = 0.01;

// PID欧拉角总误差值
float Roll_Err_Sum   = 0.0;
float Pitch_Err_Sum  = 0.0;
float Yaw_Err_Sum    = 0.0;
// PID欧拉角上一次误差值
float Roll_Err_Last  = 0.0;
float Pitch_Err_Last = 0.0;
float Yaw_Err_Last   = 0.0;

// Roll的PID计算
void PID_Roll_Calculate(void)
{
    float Proportion;
    float Integral;
    float Derivative;
    float Error, Output;

    // 外环结果输入内环作误差值
    Error = Roll_Kp * (Motor_Ail - Roll) + init_gy * 57.295780;
    
    // 总误差
    Roll_Err_Sum += Error;
    
    // PID计算
    Proportion = Roll_Rate_Kp * Error;
    Integral   = Roll_Rate_Ti * Roll_Err_Sum * Time_dt;
    Derivative = Roll_Rate_Td * (Error - Roll_Err_Last) / Time_dt;
    
    // 积分限幅
    if(Integral > INTEGRAL_MAX)
    {
        Integral = INTEGRAL_MAX;
    }
    if(Integral < INTEGRAL_MIN)
    {
        Integral = INTEGRAL_MIN;
    }
    
    // PID之和为输出
    Output = Proportion + Integral + Derivative;

    // PID输出限幅
    if(Output > PID_OUTPUT_MAX)
    {
        Output = PID_OUTPUT_MAX;
    }
    if(Output < PID_OUTPUT_MIN)
    {
        Output = PID_OUTPUT_MIN;
    }

    // 记录为上一次误差
    Roll_Err_Last = Error;
    
    // 输出PID最终值
    PID_Roll = Output;
}

// Pitch的PID计算
void PID_Pitch_Calculate(void)
{
    float Proportion;
    float Integral;
    float Derivative;
    float Error, Output;

    Error = Pitch_Kp * (Pitch - Motor_Ele) + init_gx * 57.295780;

    Pitch_Err_Sum += Error;

    Proportion = Pitch_Rate_Kp * Error;
    Integral   = Pitch_Rate_Ti * Pitch_Err_Sum * Time_dt;
    Derivative = Pitch_Rate_Td * (Error - Pitch_Err_Last) / Time_dt;

    if(Integral > INTEGRAL_MAX)
    {
        Integral = INTEGRAL_MAX;
    }
    if(Integral < INTEGRAL_MIN)
    {
        Integral = INTEGRAL_MIN;
    }
    
    Output = Proportion + Integral + Derivative;

    if(Output > PID_OUTPUT_MAX)
    {
        Output = PID_OUTPUT_MAX;
    }
    if(Output < PID_OUTPUT_MIN)
    {
        Output = PID_OUTPUT_MIN;
    }

    Pitch_Err_Last = Error;
    
    PID_Pitch = Output;
}

// Yaw的PID计算
void PID_Yaw_Calculate(void)
{
    float Proportion;
    float Integral;
    float Derivative;
    float Error, Output;

    // Yaw角不做外环，直接使用内环
    Error = init_gz * 57.295780 - Motor_Rud;
    
    Yaw_Err_Sum += Error;

    Proportion = Yaw_Rate_Kp * Error;
    Integral   = Yaw_Rate_Ti * Yaw_Err_Sum * Time_dt;
    Derivative = Yaw_Rate_Td * (Error - Yaw_Err_Last) / Time_dt;
    
    if(Integral > INTEGRAL_MAX)
    {
        Integral = INTEGRAL_MAX;
    }
    if(Integral < INTEGRAL_MIN)
    {
        Integral = INTEGRAL_MIN;
    }
    
    Output = Proportion + Integral + Derivative;

    if(Output > PID_OUTPUT_MAX)
    {
        Output = PID_OUTPUT_MAX;
    }
    if(Output < PID_OUTPUT_MIN)
    {
        Output = PID_OUTPUT_MIN;
    }

    Yaw_Err_Last = Error;
    
    PID_Yaw = Output;
}

void Motor_Calculate(void)
{
    // 获取PID更新时间
    Time_dt = Get_PID_Time();

    // 三轴PID计算
    PID_Roll_Calculate();
    PID_Pitch_Calculate();
    PID_Yaw_Calculate();

    // X模式电机转速融合公式
    // 从1-4分别为：左前顺时针、右前逆时针、左后逆时针、右后顺时针
    Motor_1 = (uint16_t)Limit_PWM(Motor_Thr - PID_Pitch - PID_Roll - PID_Yaw);
    Motor_2 = (uint16_t)Limit_PWM(Motor_Thr - PID_Pitch + PID_Roll + PID_Yaw);
    Motor_3 = (uint16_t)Limit_PWM(Motor_Thr + PID_Pitch - PID_Roll + PID_Yaw);
    Motor_4 = (uint16_t)Limit_PWM(Motor_Thr + PID_Pitch + PID_Roll - PID_Yaw);
    
    // 起飞前电机转速安全保护
    if(Motor_Thr <= 1050)
    {
        Motor_1 = 1000;
        Motor_2 = 1000;
        Motor_3 = 1000;
        Motor_4 = 1000;
    }
}

// PWM输出限幅
float Limit_PWM(float accelerator)
{
    if(accelerator > PWM_OUT_MAX)
    {
        accelerator = PWM_OUT_MAX;
    }
    else if(accelerator < PWM_OUT_MIN)
    {
        accelerator = PWM_OUT_MIN;
    }
    else
    {
        accelerator = accelerator;
    }

    return accelerator;
}

// 遥控值处理计算
void Motor_Expectation_Calculate(uint16_t ch1,uint16_t ch2,uint16_t ch3,uint16_t ch4)
{
    // 遥控值限幅
    if(ch1 < 1000) { ch1=1000; }
    if(ch1 > 2000) { ch1=2000; }

    if(ch2 < 1000) { ch2=1000; }
    if(ch2 > 2000) { ch2=2000; }

    if(ch3 < 1000) { ch3=1000; }
    if(ch3 > 2000) { ch3=2000; }

    if(ch4 < 1000) { ch4=1000; }
    if(ch4 > 2000) { ch4=2000; }

    // 三通道遥控值零偏处理及范围缩小，油门通道数值不处理
    Motor_Ail = (float)((ch1 - Ail_Mid) * 0.06);
    Motor_Ele = (float)((ch2 - Ele_Mid) * 0.06);
    Motor_Thr = (float)ch3;
    Motor_Rud = (float)((ch4 - Rud_Mid) * 0.10);
}
