#include "stm32f4xx.h"
#include "delay.h"
#include "ucos_ii.h"

static u8  fac_us = 0;
static u16 fac_ms = 0;

// 延时设置，SYSCLK为当前时钟频率（MHZ）
void Delay_Config(u8 SYSCLK)
{
// 若定义则使用ucosII
#ifdef OS_CRITICAL_METHOD
    u32 reload;
#endif

    // 系统定时器配置，八分频
    SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
    // 获取us值
    fac_us = SYSCLK / 8;

// 若定义则使用ucosII
#ifdef OS_CRITICAL_METHOD
    // 每秒计数次数（K）
    reload = SYSCLK / 8;
    // 设定溢出时间
    reload *= 1000000 / OS_TICKS_PER_SEC;
    // ucosII可延时的最小单位
    fac_ms = 1000 / OS_TICKS_PER_SEC;
    // 开启Systick中断
    SysTick->CTRL |= SysTick_CTRL_TICKINT_Msk;
    // 每(1 / OS_TICKS_PER_SEC)秒中断一次
    SysTick->LOAD  = reload;
    // 开启Systick
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
#else
    // 获取非ucosII下的ms值
    fac_ms = (u16)fac_us * 1000;
#endif
}

// 若定义则使用ucosII
#ifdef OS_CRITICAL_METHOD
// 延时us，nus为要延时的us数
void delay_us(u32 nus)
{
    u32 ticks;
    u32 told, tnow, tcnt;
    
    // 装载值
    u32 reload = SysTick->LOAD;
    // 节拍数
    ticks = nus * fac_us;
    tcnt = 0;
    // 操作系统调度上锁
    OSSchedLock();
    // 当前SysTick值
    told = SysTick->VAL;
    
    while(1)
    {
        // 当前SysTick值
        tnow = SysTick->VAL;
        if(tnow != told)
        {
            // 递减计数
            if(tnow < told)
            {
                tcnt += told - tnow;
            }
            else
            {
                // 定时器记过一轮，补偿reload
                tcnt += reload - tnow + told;
            }
            told = tnow;
            // 超时
            if(tcnt >= ticks)
            {
                break;
            }
        }
    };

    // 操作系统调度解锁
    OSSchedUnlock();
}

// 延时ms，nms为要延时的ms数
void delay_ms(u16 nms)
{
    // 若操作系统正在运行
    if(OSRunning == OS_TRUE && OSLockNesting == 0)
    {
        // 延时的时间大于操作系统最小单位
        if(nms >= fac_ms)
        {
            // 操作系统延时
            OSTimeDly(nms / fac_ms);
        }
        // 延时的时间大于操作系统最小单位，采用普通方法延时
        nms %= fac_ms;
    }
    // 普通方法延时
    delay_us((u32)(nms * 1000));
}

// 非使用ucosII
#else
// 延时us，nus为要延时的us数
void delay_us(u32 nus)
{
    u32 temp;
    // 时间加载
    SysTick->LOAD  = nus * fac_us;
    // 清空计时器
    SysTick->VAL   = 0x00;
    // 开启Systick
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

    // 开始计时直到时间到达
    do
    {
        temp = SysTick->CTRL;
    }
    while((temp & 0x01) && !(temp & (1<<16)));

    // 关闭Systick
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
    // 清空计时器
    SysTick->VAL   = 0X00; 
}

// 延时ms，nms为要延时的ms数
void delay_ms(u16 nms)
{
    u32 temp;

    // 时间加载
    SysTick->LOAD  = (u32)nms * fac_ms;
    // 清空计数器
    SysTick->VAL   = 0x00;
    // 开启Systick
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;

    // 开始计时直到时间到达
    do
    {
        temp = SysTick->CTRL;
    }
    while((temp & 0x01) && !(temp & (1<<16)));

    // 关闭Systick
    SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;
    // 清空计时器
    SysTick->VAL =0X00;
}
#endif
