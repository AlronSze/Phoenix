#include <includes.h>

// Board clock frequency setting. Copied from STM32 examples.
void SysClk_Config()
{
    RCC->CR |= ((uint32_t)RCC_CR_HSION);                     // Enable HSI
    while ((RCC->CR & RCC_CR_HSIRDY) == 0);                  // Wait for HSI Ready

    RCC->CFGR = RCC_CFGR_SW_HSI;                             // HSI is system clock
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_HSI);  // Wait for HSI used as system clock

    FLASH->ACR  = FLASH_ACR_PRFTEN;                          // Enable Prefetch Buffer
    FLASH->ACR |= FLASH_ACR_ICEN;                            // Instruction cache enable
    FLASH->ACR |= FLASH_ACR_DCEN;                            // Data cache enable
    FLASH->ACR |= FLASH_ACR_LATENCY_5WS;                     // Flash 5 wait state

    RCC->CFGR |= RCC_CFGR_HPRE_DIV1;                         // HCLK = SYSCLK
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV2;                        // APB1 = HCLK/2
    RCC->CFGR |= RCC_CFGR_PPRE2_DIV1;                        // APB2 = HCLK/1

    RCC->CR &= ~RCC_CR_PLLON;                                // Disable PLL

    // PLL configuration:  VCO = HSI/M * N,  Sysclk = VCO/P
    RCC->PLLCFGR = ( 16ul                   |                // PLL_M =  16
                   (320ul <<  6)            |                // PLL_N = 320
                   (  1ul << 16)            |                // PLL_P =   4
                   (RCC_PLLCFGR_PLLSRC_HSI) |                // PLL_SRC = HSI
                   (  8ul << 24)             );              // PLL_Q =   8

    RCC->CR |= RCC_CR_PLLON;                                 // Enable PLL
    while((RCC->CR & RCC_CR_PLLRDY) == 0) __NOP();           // Wait till PLL is ready

    RCC->CFGR &= ~RCC_CFGR_SW;                               // Select PLL as system clock source
    RCC->CFGR |=  RCC_CFGR_SW_PLL;
    while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);  // Wait till PLL is system clock src
}

// LED initialization
void Led_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;

    // Enable GPIOA's clock
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);

    // GPIO configuration. Push-pull output
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA,&GPIO_InitStructure);
}

// Board support package initialization
void BSP_Init(void)
{
    // System clock configuration
    SysClk_Config();
    SystemCoreClockUpdate();

    // Delay configuration
    Delay_Config(80);

    // LED initialization
    Led_Init();
    // USART initialization
    USART2_Init();
    // I2C initialization
    I2C1_Init();

    // MPU initialization
    MPU6050_Init();
    HMC5883L_Init();

    // AHRS timer initialization
    AHRS_Time_Init();
    // Quaternion initialization
    Quat_Init();

    // PWM output and input initialization.
    // Put the initialization between delays to prevent errors
    delay_ms(1);
    PWM_Out_Init();
    PWM_In_Init();
    delay_ms(1);

    // PID Timer initialization
    PID_Time_Init();

    // Turn on the LED to indicate all initializations is completed
    GPIO_SetBits(GPIOA, GPIO_Pin_5);
}
