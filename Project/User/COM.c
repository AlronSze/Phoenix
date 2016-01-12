#include "stm32f4xx_it.h"
#include "COM.h"
#include "PID.h"
#include "USART.h"
#include "delay.h"

// 上位机命令存放数组
uint8_t ComRxBuffer[4] = {0};

// 获取上位机命令
void Get_COM(void)
{
	// 命令帧格式鉴别
	if (ComRxBuffer[0] == 0x8A && ComRxBuffer[1] == 0xFE && ComRxBuffer[3] == 0xFC)
	{
		// 命令识别示范，因调节完PID后此通信功能暂不使用，等待未来加入
		if (ComRxBuffer[2] == 0x01)
		{
			// 此处加入功能
		}
		else if (ComRxBuffer[2] == 0x02)
		{
			// 此处加入功能
		}
		
		// 命令清除
		ComRxBuffer[0] = 0;
		ComRxBuffer[1] = 0;
		ComRxBuffer[2] = 0;
		ComRxBuffer[3] = 0;
	}
}
