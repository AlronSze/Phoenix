#include "TIM.h"
#include "USART.h"

// 四通道遥控初始值
uint16_t PWMInCh1 = 0, PWMInCh2 = 0, PWMInCh3 = 0, PWMInCh4 = 0;

// PWM输出初始化
void PWM_Out_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_OCInitTypeDef TIM_OCInitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	// 时钟开启
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	
	// GPIO配置，推挽复用
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	// GPIO复用TIM1
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource8, GPIO_AF_TIM1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource9, GPIO_AF_TIM1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10, GPIO_AF_TIM1);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource11, GPIO_AF_TIM1);

	// 关闭TIM1
	TIM_DeInit(TIM1);
	// TIM1配置，预分频为80，周期为2500，向上计数
	TIM_TimeBaseStructure.TIM_Period = 2500 - 1;
	TIM_TimeBaseStructure.TIM_Prescaler = 80 - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

	// TIM1输出配置，PWM输出模式，初始装载1000
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1;
	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
	TIM_OCInitStructure.TIM_Pulse = 1000;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
	
	// TIM1四个通道使能及预装载
	TIM_OC1Init(TIM1, &TIM_OCInitStructure);
	TIM_OC1PreloadConfig(TIM1, TIM_OCPreload_Enable);
	TIM_OC2Init(TIM1, &TIM_OCInitStructure);
	TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
	TIM_OC3Init(TIM1, &TIM_OCInitStructure);
	TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
	TIM_OC4Init(TIM1, &TIM_OCInitStructure);
	TIM_OC4PreloadConfig(TIM1, TIM_OCPreload_Enable);
	
	// 开启ARR预装载缓冲器
	TIM_ARRPreloadConfig(TIM1, ENABLE);
	
	// 使能TIM的PWM输出
	TIM_Cmd(TIM1, ENABLE); 
	TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

// PWM输出给四个电调
void PWM_Output(uint16_t DR1,uint16_t DR2,uint16_t DR3,uint16_t DR4)
{
	TIM_SetCompare1(TIM1, DR1);
	TIM_SetCompare2(TIM1, DR2);
	TIM_SetCompare3(TIM1, DR3);
	TIM_SetCompare4(TIM1, DR4);
}

// PWM输入捕获初始化
void PWM_In_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	TIM_ICInitTypeDef TIM_ICInitStructure;
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

	// 时钟开启
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
	
	// GPIO配置，推挽复用
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	
	// GPIO复用TIM3
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource6,GPIO_AF_TIM3);
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource7,GPIO_AF_TIM3);
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource8,GPIO_AF_TIM3);
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource9,GPIO_AF_TIM3);

	// 关闭TIM3
	TIM_DeInit(TIM3);
	// TIM3配置，预分频为80，周期为0xFFFF，向上计数
	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
	TIM_TimeBaseStructure.TIM_Prescaler = 80 - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

	// TIM3四个通道输入配置，捕获上升沿，滤波值为0x0B
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0B;
	TIM_ICInit(TIM3, &TIM_ICInitStructure);
	
	TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0B;
	TIM_ICInit(TIM3, &TIM_ICInitStructure);

	TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0B;
	TIM_ICInit(TIM3, &TIM_ICInitStructure);

	TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
	TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
	TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
	TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
	TIM_ICInitStructure.TIM_ICFilter = 0x0B;
	TIM_ICInit(TIM3, &TIM_ICInitStructure);

	// NVIC配置
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

	// 开启TIM3四个通道的中断
	TIM_ITConfig(TIM3,TIM_IT_CC1,ENABLE);
	TIM_ITConfig(TIM3,TIM_IT_CC2,ENABLE);
	TIM_ITConfig(TIM3,TIM_IT_CC3,ENABLE);
	TIM_ITConfig(TIM3,TIM_IT_CC4,ENABLE);

	// 使能TIM3
	TIM_Cmd(TIM3, ENABLE); 
}

// PID更新时间所用定时器初始化
void PID_Time_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	
	// 时钟开启
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);
	
	// 关闭TIM4
	TIM_DeInit(TIM4);
	// TIM4配置，预分频为80，周期为0xFFFF，向上计数
	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
	TIM_TimeBaseStructure.TIM_Prescaler = 80 - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
	
	// 使能TIM4
	TIM_Cmd(TIM4, ENABLE);
}

// AHRS更新时间所用定时器初始化
void AHRS_Time_Init(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	
	// 时钟开启
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);
	
	// 关闭TIM5
	TIM_DeInit(TIM5);
	// TIM5配置，预分频为80，周期为0xFFFF，向上计数
	TIM_TimeBaseStructure.TIM_Period = 0xFFFF;
	TIM_TimeBaseStructure.TIM_Prescaler = 80 - 1;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM5, &TIM_TimeBaseStructure);
	
	// 使能TIM5
	TIM_Cmd(TIM5, ENABLE);
}

// 获取PID更新时间
float Get_PID_Time(void)
{
	float temp = 0;
	static uint32_t now = 0;

	// 时间获取
 	now = TIM4->CNT;
	// 定时器清零
  TIM4->CNT = 0;
	// 转化单位为HZ
	temp = (float)now / 1000000.0f;

	return temp;
}

// 获取AHRS更新时间的一半
float Get_AHRS_Time(void)
{
	float temp = 0;
	static uint32_t now = 0;

	// 时间获取
	now = TIM5->CNT;
	// 定时器清零
	TIM5->CNT = 0;
	// 转化单位为HZ并取一半
	temp = (float)now / 2000000.0f;

	return temp;
}
