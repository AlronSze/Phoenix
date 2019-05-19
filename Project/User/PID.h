#include "stm32f4xx.h"

// Extern variables
extern uint16_t Motor_1, Motor_2, Motor_3, Motor_4;
extern float Alt_Err_Sum;
extern float Alt_Err_Last;

// Functions definition
void Motor_Calculate(void);
float Limit_PWM(float accelerator);
void Once_Motor_Mid_Cal(uint16_t ch1, uint16_t ch2, uint16_t ch4);
void Motor_Middle_Calculate(void);
void Motor_Expectation_Calculate(uint16_t ch1,uint16_t ch2,uint16_t ch3,uint16_t ch4);
