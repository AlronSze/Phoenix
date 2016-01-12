#include "stm32f4xx.h"

// 变量链接
extern uint16_t PWMInCh1, PWMInCh2, PWMInCh3, PWMInCh4;

// 函数定义
void PWM_Out_Init(void);
void PWM_Output(uint16_t DR1,uint16_t DR2,uint16_t DR3,uint16_t DR4);
void PWM_In_Init(void);
void PID_Time_Init(void);
void AHRS_Time_Init(void);
float Get_PID_Time(void);
float Get_AHRS_Time(void);
