#include "delay.h"
#include "math.h"
#include "PID.h"
#include "AHRS.h"
#include "TIM.h"
#include "USART.h"
#include "MPU6050.h"
#include "COM.h"

// Integral limit
#define INTEGRAL_MAX     200.0f
#define INTEGRAL_MIN     -200.0f

// PID out limit
#define PID_OUTPUT_MAX   800.0f
#define PID_OUTPUT_MIN   -800.0f

// PWM out limit
#define PWM_OUT_MIN      875
#define PWM_OUT_MAX      2000

// PID update time
float Time_dt;
// Euler angle PID output value
float PID_Roll, PID_Pitch, PID_Yaw;
// Motor speed
uint16_t Motor_1, Motor_2, Motor_3, Motor_4;

// Remote control value
float Motor_Ail = 0.0;
float Motor_Ele = 0.0;
float Motor_Thr = 0.0;
float Motor_Rud = 0.0;
// Remote control median value
float Ail_Mid   = 1527.0;
float Ele_Mid   = 1522.0;
float Rud_Mid   = 1528.0;

// Roll PID parameters
float Roll_Kp        = 1.9;
float Roll_Rate_Kp   = 0.70;
float Roll_Rate_Ti   = 0.10;
float Roll_Rate_Td   = 0.01;
// Pitch PID parameters
float Pitch_Kp       = 2.4;
float Pitch_Rate_Kp  = 0.70;
float Pitch_Rate_Ti  = 0.10;
float Pitch_Rate_Td  = 0.01;
// Yaw PID parameters
float Yaw_Kp         = 0.0;
float Yaw_Rate_Kp    = 0.70;
float Yaw_Rate_Ti    = 0.10;
float Yaw_Rate_Td    = 0.01;

// The sum of euler angle PID error
float Roll_Err_Sum   = 0.0;
float Pitch_Err_Sum  = 0.0;
float Yaw_Err_Sum    = 0.0;
// Last euler angle PID error
float Roll_Err_Last  = 0.0;
float Pitch_Err_Last = 0.0;
float Yaw_Err_Last   = 0.0;

// Roll PID calculation
void PID_Roll_Calculate(void)
{
    float Proportion;
    float Integral;
    float Derivative;
    float Error, Output;

    // Outer ring result is input to the inner ring as the error value
    Error = Roll_Kp * (Motor_Ail - Roll) + init_gy * 57.295780;
    
    // The sum of error
    Roll_Err_Sum += Error;
    
    // PID calculation
    Proportion = Roll_Rate_Kp * Error;
    Integral   = Roll_Rate_Ti * Roll_Err_Sum * Time_dt;
    Derivative = Roll_Rate_Td * (Error - Roll_Err_Last) / Time_dt;
    
    // Integral limit
    if(Integral > INTEGRAL_MAX)
    {
        Integral = INTEGRAL_MAX;
    }
    if(Integral < INTEGRAL_MIN)
    {
        Integral = INTEGRAL_MIN;
    }
    
    // P + I + D = output
    Output = Proportion + Integral + Derivative;

    // PID output limit
    if(Output > PID_OUTPUT_MAX)
    {
        Output = PID_OUTPUT_MAX;
    }
    if(Output < PID_OUTPUT_MIN)
    {
        Output = PID_OUTPUT_MIN;
    }

    // Record last error
    Roll_Err_Last = Error;
    
    // Output PID value
    PID_Roll = Output;
}

// Pitch PID calculation
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

// Yaw PID calculation
void PID_Yaw_Calculate(void)
{
    float Proportion;
    float Integral;
    float Derivative;
    float Error, Output;

    // Yaw does not need outer ring, so use inner ring directly
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
    // Get PID update time
    Time_dt = Get_PID_Time();

    // XYZ PID calculation
    PID_Roll_Calculate();
    PID_Pitch_Calculate();
    PID_Yaw_Calculate();

    // Motor speed fusion formula in X mode
    // From 1-4 are: clockwise left front, counterclockwise right front, counterclockwise left rear, clockwise right rear
    Motor_1 = (uint16_t)Limit_PWM(Motor_Thr - PID_Pitch - PID_Roll - PID_Yaw);
    Motor_2 = (uint16_t)Limit_PWM(Motor_Thr - PID_Pitch + PID_Roll + PID_Yaw);
    Motor_3 = (uint16_t)Limit_PWM(Motor_Thr + PID_Pitch - PID_Roll + PID_Yaw);
    Motor_4 = (uint16_t)Limit_PWM(Motor_Thr + PID_Pitch + PID_Roll - PID_Yaw);
    
    // Motor speed safety protection before takeoff
    if(Motor_Thr <= 1050)
    {
        Motor_1 = 1000;
        Motor_2 = 1000;
        Motor_3 = 1000;
        Motor_4 = 1000;
    }
}

// PWM output limit
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

// Remote control value calculation
void Motor_Expectation_Calculate(uint16_t ch1, uint16_t ch2, uint16_t ch3, uint16_t ch4)
{
    // Remote control value limit
    if(ch1 < 1000) { ch1=1000; }
    if(ch1 > 2000) { ch1=2000; }

    if(ch2 < 1000) { ch2=1000; }
    if(ch2 > 2000) { ch2=2000; }

    if(ch3 < 1000) { ch3=1000; }
    if(ch3 > 2000) { ch3=2000; }

    if(ch4 < 1000) { ch4=1000; }
    if(ch4 > 2000) { ch4=2000; }

    // Three-channel remote control value zero offset processing and range reduction
    // (Throttle channel value is not processed)
    Motor_Ail = (float)((ch1 - Ail_Mid) * 0.06);
    Motor_Ele = (float)((ch2 - Ele_Mid) * 0.06);
    Motor_Thr = (float)ch3;
    Motor_Rud = (float)((ch4 - Rud_Mid) * 0.10);
}
