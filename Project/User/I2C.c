#include "stm32f4xx.h"
#include "stdio.h"
#include "USART.h"
#include "delay.h"
#include "I2C.h"

// I2C初始化，使用模拟I2C
void I2C1_Init(void)
{
	GPIO_InitTypeDef  GPIO_InitStructure;
	
	// 时钟开启
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB,ENABLE);

	// GPIO配置，开漏输出
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT; 
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_Speed = GPIO_High_Speed;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; 
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

// I2C模拟开始
u8 I2C_START(void)
{ 
	I2C_SDA_H; 
 	I2C_NOP;

 	I2C_SCL_H; 
 	I2C_NOP;    

 	if(!I2C_SDA_STATE) 
		return I2C_BUS_BUSY;

 	I2C_SDA_L;
 	I2C_NOP;

 	I2C_SCL_L;  
 	I2C_NOP; 

 	if(I2C_SDA_STATE) 
		return I2C_BUS_ERROR;

 	return I2C_READY;
}

// I2C模拟停止
void I2C_STOP(void)
{
 	I2C_SDA_L; 
 	I2C_NOP;

 	I2C_SCL_H; 
 	I2C_NOP;    

 	I2C_SDA_H;
 	I2C_NOP;
}

// I2C模拟发送ACK
void I2C_SendACK(void)
{
 	I2C_SDA_L;
 	I2C_NOP;
 	I2C_SCL_H;
 	I2C_NOP;
 	I2C_SCL_L; 
 	I2C_NOP;  
}

// I2C模拟发送NACK
void I2C_SendNACK(void)
{
	I2C_SDA_H;
	I2C_NOP;
	I2C_SCL_H;
	I2C_NOP;
	I2C_SCL_L; 
	I2C_NOP;  
}

// I2C模拟发送一个字节
u8 I2C_SendByte(u8 i2c_data)
{
 	u8 i;
 	
	I2C_SCL_L;
 	for(i=0;i<8;i++)
 	{  
  		if(i2c_data&0x80) 
				I2C_SDA_H;
   		else 
				I2C_SDA_L;

  		i2c_data<<=1;
  		I2C_NOP;

  		I2C_SCL_H;
  		I2C_NOP;
  		I2C_SCL_L;
  		I2C_NOP; 
 	}

 	I2C_SDA_H; 
 	I2C_NOP;
 	I2C_SCL_H;
 	I2C_NOP;   
 	if(I2C_SDA_STATE)
 	{
  		I2C_SCL_L;
  		return I2C_NACK;
 	}
 	else
 	{
  		I2C_SCL_L;
  		return I2C_ACK;  
 	}    
}

// I2C模拟接收一个字节
u8 I2C_ReceiveByte(void)
{
	u8 i,i2c_data;

 	I2C_SDA_H;
 	I2C_SCL_L; 
 	i2c_data=0;

 	for(i=0;i<8;i++)
 	{
  		I2C_SCL_H;
  		I2C_NOP; 
  		i2c_data<<=1;

  		if(I2C_SDA_STATE)	
				i2c_data|=0x01; 
  
  		I2C_SCL_L;  
  		I2C_NOP;         
 	}
	I2C_SendNACK();
 	return i2c_data;
}

// I2C模拟接收一个字节并发送ACK
u8 I2C_ReceiveByte_WithACK(void)
{
	u8 i,i2c_data;

 	I2C_SDA_H;
 	I2C_SCL_L; 
 	i2c_data=0;

 	for(i=0;i<8;i++)
 	{
  		I2C_SCL_H;
  		I2C_NOP; 
  		i2c_data<<=1;

  		if(I2C_SDA_STATE)	
				i2c_data|=0x01; 
  
  		I2C_SCL_L;  
  		I2C_NOP;         
 	}
	I2C_SendACK();
 	return i2c_data;
}

// I2C模拟写一个字节
void I2C_WriteByte(uint8_t DeviceAddr, uint8_t address, uint8_t data)
{
   I2C_START();
   I2C_SendByte(DeviceAddr);
   I2C_SendByte(address);
   I2C_SendByte(data);
   I2C_STOP();
}

// I2C模拟写入一个字节（无需寄存器地址）
void I2C_NoAddr_WriteByte(uint8_t DeviceAddr, uint8_t data)
{
	I2C_START();
	I2C_SendByte(DeviceAddr);
	I2C_SendByte(data);
	I2C_STOP();
}

// I2C模拟读出一个字节
uint8_t I2C_ReadByte(uint8_t DeviceAddr, uint8_t address)
{
   uint8_t i;
   I2C_START();
   I2C_SendByte(DeviceAddr);
   I2C_SendByte(address);
   I2C_START();
   I2C_SendByte(DeviceAddr + 1);
   i = I2C_ReceiveByte();
   I2C_STOP();
   return i;
}

// I2C模拟读出两个字节
uint16_t I2C_Read_2Bytes(uint8_t DeviceAddr, uint8_t address)
{
	uint8_t data_temp1,data_temp2;
	uint16_t data_16;

	I2C_START();
	I2C_SendByte(DeviceAddr);
	I2C_SendByte(address);
	I2C_START();
	I2C_SendByte(DeviceAddr + 1);
	data_temp1 = I2C_ReceiveByte_WithACK();	
	data_temp2 = I2C_ReceiveByte();
	I2C_STOP();
	
	data_16 = (data_temp1<<8) | data_temp2;
	return data_16;
}

// I2C模拟读出三个字节
uint32_t I2C_Read_3Bytes(uint8_t DeviceAddr, uint8_t address)
{
	uint8_t data_temp1, data_temp2, data_temp3;
	uint32_t data_32;

	I2C_START();
	I2C_SendByte(DeviceAddr);
	I2C_SendByte(address);
	I2C_START();
	I2C_SendByte(DeviceAddr + 1);
	data_temp1 = I2C_ReceiveByte_WithACK();
	data_temp2 = I2C_ReceiveByte_WithACK();	
	data_temp3 = I2C_ReceiveByte();
	I2C_STOP();
	
	data_32 = (data_temp1<<16) | (data_temp2<<8) | data_temp3;
	return data_32;
}

// I2C模拟写入多个字节
u8 i2cwrite(u8 dev_addr, u8 reg_addr, u8 i2c_len, u8 *i2c_data_buf)
{		
		u8 i;
		I2C_START();
		I2C_SendByte(dev_addr << 1 | I2C_Direction_Transmitter);
		I2C_SendByte(reg_addr);
		for (i = 0; i < i2c_len; i++) 
	      I2C_SendByte(i2c_data_buf[i]);
	
		I2C_STOP();
		return 0x00;
}

// I2C模拟读出多个字节
u8 i2cread(u8 dev_addr, u8 reg_addr, u8 i2c_len, u8 *i2c_data_buf)
{
		I2C_START();
		I2C_SendByte(dev_addr << 1 | I2C_Direction_Transmitter);
		I2C_SendByte(reg_addr);
		I2C_START();
		I2C_SendByte(dev_addr << 1 | I2C_Direction_Receiver);

    while (i2c_len) 
		{
			if (i2c_len == 1) 
				*i2c_data_buf = I2C_ReceiveByte();  
      else 
				*i2c_data_buf = I2C_ReceiveByte_WithACK();
      i2c_data_buf++;
      i2c_len--;
    }
		I2C_STOP();
    return 0x00;
}
