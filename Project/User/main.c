// 头文件
#include <includes.h>

// 堆栈分配
static OS_STK Task_Startup_STK[TASK_STARTUP_STK_SIZE];
static OS_STK Task_Angel_STK[TASK_ANGEL_STK_SIZE];
static OS_STK Task_PID_STK[TASK_PID_STK_SIZE];
static OS_STK Task_COM_STK[TASK_COM_STK_SIZE];

// 函数定义
static void OS_Systick_Init(void);
static void Task_Startup(void *p_arg);
static void Task_Angel(void *p_arg);
static void Task_PID(void *p_arg);
static void Task_COM(void *p_arg);

// 主函数
int main(void)
{
    // 操作系统初始化
    OSInit();
    // 创建启动任务
    OSTaskCreate(Task_Startup, (void *)0, &Task_Startup_STK[TASK_STARTUP_STK_SIZE - 1], TASK_STARTUP_PRIO);
    // 运行操作系统
    OSStart();
    
    return 0;
}

// 操作系统Systick初始化
static void OS_Systick_Init(void)
{
    RCC_ClocksTypeDef rcc_clocks;
    RCC_GetClocksFreq(&rcc_clocks);
    SysTick_Config(rcc_clocks.HCLK_Frequency / OS_TICKS_PER_SEC);
}

// 启动任务函数
static void Task_Startup(void *p_arg)
{
    // 板级驱动初始化
    BSP_Init();
    // 操作系统Systick初始化
    OS_Systick_Init();

    // 检测CPU当前容量
    #if (OS_TASK_STAT_EN > 0)
        OSStatInit();
    #endif

    // 创建功能任务
    OSTaskCreate(Task_Angel, (void *)0, &Task_Angel_STK[TASK_ANGEL_STK_SIZE - 1], TASK_ANGEL_PRIO);
    OSTaskCreate(Task_PID, (void *)0, &Task_PID_STK[TASK_PID_STK_SIZE - 1], TASK_PID_PRIO);
    OSTaskCreate(Task_COM, (void *)0, &Task_COM_STK[TASK_COM_STK_SIZE - 1], TASK_COM_PRIO);

    // 删除自身
    OSTaskDel(OS_PRIO_SELF);
}

// 姿态解算任务
static void Task_Angel(void *p_arg)
{
    while(1)
    {
        // 获取传感器数据
        Get_AHRS_Data();
        // 姿态更新
        AHRS_Update(init_gx, init_gy, init_gz, init_ax, init_ay, init_az, init_mx, init_my, init_mz);
        // 延时
        OSTimeDly(1);
    }
}

// PID控制任务
static void Task_PID(void *p_arg)
{
    while(1)
    {
        // 遥控值处理
        Motor_Expectation_Calculate(PWMInCh1, PWMInCh2, PWMInCh3, PWMInCh4);
        // 电机PID计算
        Motor_Calculate();
        // 输出控制电机
        PWM_Output(Motor_1, Motor_2, Motor_3, Motor_4);
        // 延时
        OSTimeDly(5);
    }
}

// 蓝牙/串口通信任务
static void Task_COM(void *p_arg)
{
    while(1)
    {
        // 获取上位机命令
        Get_COM();
        // 延时
        OSTimeDly(20);
    }
}
