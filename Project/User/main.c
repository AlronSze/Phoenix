// Header
#include <includes.h>

// Stack allocation
static OS_STK Task_Startup_STK[TASK_STARTUP_STK_SIZE];
static OS_STK Task_Angel_STK[TASK_ANGEL_STK_SIZE];
static OS_STK Task_PID_STK[TASK_PID_STK_SIZE];
static OS_STK Task_COM_STK[TASK_COM_STK_SIZE];

// Functions definition
static void OS_Systick_Init(void);
static void Task_Startup(void *p_arg);
static void Task_Angel(void *p_arg);
static void Task_PID(void *p_arg);
static void Task_COM(void *p_arg);

// Main function
int main(void)
{
    // OS initialization
    OSInit();
    // Create startup task
    OSTaskCreate(Task_Startup, (void *)0, &Task_Startup_STK[TASK_STARTUP_STK_SIZE - 1], TASK_STARTUP_PRIO);
    // Run OS
    OSStart();

    return 0;
}

// OS systick initialization
static void OS_Systick_Init(void)
{
    RCC_ClocksTypeDef rcc_clocks;
    RCC_GetClocksFreq(&rcc_clocks);
    SysTick_Config(rcc_clocks.HCLK_Frequency / OS_TICKS_PER_SEC);
}

// Function entry of startup task
static void Task_Startup(void *p_arg)
{
    // Board support package initialization
    BSP_Init();
    // OS systick initialization
    OS_Systick_Init();

    // Detect OS task current capacity
    #if (OS_TASK_STAT_EN > 0)
        OSStatInit();
    #endif

    // Create functional task
    OSTaskCreate(Task_Angel, (void *)0, &Task_Angel_STK[TASK_ANGEL_STK_SIZE - 1], TASK_ANGEL_PRIO);
    OSTaskCreate(Task_PID, (void *)0, &Task_PID_STK[TASK_PID_STK_SIZE - 1], TASK_PID_PRIO);
    OSTaskCreate(Task_COM, (void *)0, &Task_COM_STK[TASK_COM_STK_SIZE - 1], TASK_COM_PRIO);

    // Delete itself
    OSTaskDel(OS_PRIO_SELF);
}

// Function entry of AHRS task
static void Task_Angel(void *p_arg)
{
    while(1)
    {
        // Get sensor data
        Get_AHRS_Data();
        // Update attitude
        AHRS_Update(init_gx, init_gy, init_gz, init_ax, init_ay, init_az, init_mx, init_my, init_mz);
        // OS time delay 1 tick
        OSTimeDly(1);
    }
}

// Function entry of PID control task
static void Task_PID(void *p_arg)
{
    while(1)
    {
        // Remote control value processing
        Motor_Expectation_Calculate(PWMInCh1, PWMInCh2, PWMInCh3, PWMInCh4);
        // Motor PID calculation
        Motor_Calculate();
        // Output PWM to motors
        PWM_Output(Motor_1, Motor_2, Motor_3, Motor_4);
        // OS time delay 5 ticks
        OSTimeDly(5);
    }
}

// Function entry of bluetooth/serial communication task
static void Task_COM(void *p_arg)
{
    while(1)
    {
        // Get the command from upper computer
        Get_COM();
        // OS time delay 20 ticks
        OSTimeDly(20);
    }
}
