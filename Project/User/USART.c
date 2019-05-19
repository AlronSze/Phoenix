#include "USART.h"

// Printf function overwrite
int fputc(int ch, FILE *f)
{
    USART_SendData(USART2, (unsigned char) ch);
    while (!(USART2->SR & USART_FLAG_TXE));
    return (ch);
}
int GetKey (void)
{
    while (!(USART2->SR & USART_FLAG_RXNE));
    return ((int)(USART2->DR & 0x1FF));
}

// USART2 initialization
void USART2_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    
    // Enable USART2 and GPIOA clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    
    // GPIO alternate function configuration
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2);

    // Rx GPIO configuration. Push-pull alternate function
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // Tx GPIO configuration. Open-drain
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    // USART configuration (115200 8n1)
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init(USART2, &USART_InitStructure);

    // Rx interrupt configuration
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);

    // Enable USART
    USART_Cmd(USART2, ENABLE);

    // NVIC initialization
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

// USART send one character
void usart_send_char(u8 c)
{
    while(USART_GetFlagStatus(USART2, USART_FLAG_TC) == RESET);
    USART_SendData(USART2, c);
}

// USART data report by ANO-Tech upper computer software
void usart_report(u8 fun, u8* data, u8 len)
{
    u8 send_buf[32] = {0};
    u8 i;
    
    if(len > 28)
        return;

    send_buf[len + 3] = 0;
    send_buf[0] = 0X88;
    send_buf[1] = fun;
    send_buf[2] = len;

    for(i = 18; i <= 23; i++)
        send_buf[3 + i] = data[i];
    for(i = 0; i < len + 3; i++)
        send_buf[len + 3] += send_buf[i];
    for(i = 0; i< len + 4; i++)
        usart_send_char(send_buf[i]);
}

// USART imu data report by ANO-Tech upper computer software
void usart_report_imu(short roll, short pitch, short yaw)
{
    u8 tbuf[28] = {0}; 

    tbuf[18] = (roll >> 8) & 0XFF;
    tbuf[19] = roll & 0XFF;
    tbuf[20] = (pitch >> 8) & 0XFF;
    tbuf[21] = pitch & 0XFF;
    tbuf[22] = (yaw >> 8) & 0XFF;
    tbuf[23] = yaw & 0XFF;
    
    usart_report(0XAF, tbuf, 28);
}
