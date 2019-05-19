#include "stm32f4xx.h"
#include "stdio.h"

// Functions definition
int fputc(int ch, FILE *f);
int GetKey(void);

void USART2_Init(void);

void usart_send_char(u8 c);
void usart_report(u8 fun, u8* data, u8 len);
void usart_report_imu(short roll, short pitch, short yaw);
