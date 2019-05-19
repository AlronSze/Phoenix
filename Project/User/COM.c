#include "stm32f4xx_it.h"
#include "COM.h"
#include "PID.h"
#include "USART.h"
#include "delay.h"

// Array for storing command from host
uint8_t ComRxBuffer[4] = {0};

// Get the command from upper computer
void Get_COM(void)
{
    // Command frame checking
    if (ComRxBuffer[0] == 0x8A && ComRxBuffer[1] == 0xFE && ComRxBuffer[3] == 0xFC)
    {
        // Temporarily useless. Wait to add functions in the future
        if (ComRxBuffer[2] == 0x01)
        {
            // TODO: Add some functions
        }
        else if (ComRxBuffer[2] == 0x02)
        {
            // TODO: Add some functions
        }
        
        // Clear command
        ComRxBuffer[0] = 0;
        ComRxBuffer[1] = 0;
        ComRxBuffer[2] = 0;
        ComRxBuffer[3] = 0;
    }
}
