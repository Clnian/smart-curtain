/**
  ******************************************************************************
  * @file    usart3.c
  * @brief   USART1调试串口驱动。文件名沿用usart3，实际使用USART1。
  * @note    USART1使用PA9/PA10，与电机PA9/PA10存在复用冲突；如启用调试串口，需避开电机运行。
  * 作者：程磊
  * 时间：2026-04-27 16:35:27 +08:00
  ******************************************************************************
  */

#include "usart3.h"

/**
  * @brief  初始化USART1调试串口。
  * @param  BRR 波特率，例如115200。
  */
void USART1_Config(uint32_t BRR)
{
	/* 1. 使能GPIOA和USART1时钟 */
	RCC->APB2ENR |= (1<<2);
	RCC->APB2ENR |= (1<<14);

	/* 2. 配置PA9为复用推挽输出，PA10为浮空输入 */
	GPIOA->CRH &=~(0xF<<4);
	GPIOA->CRH |=(0xB<<4);
	GPIOA->CRH &=~(0xF<<8);
	GPIOA->CRH |=(0x4<<8);

	/* 3. 配置USART1：8位数据位、1位停止位、无校验 */
	USART1->CR1 &=~(0X1<<12);
	USART1->CR2 &=~(0X3<<12);

	USART1->BRR = 72000000/BRR;

	/* 4. 使能收发、RXNE中断和IDLE中断 */
	USART1->CR1 |=(0x1<<2);
	USART1->CR1 |=(0x1<<3);
	USART1->CR1 |=(0x1<<4);
	USART1->CR1 |=(0x1<<5);

	NVIC_EnableIRQ(USART1_IRQn);
	NVIC_SetPriority(USART1_IRQn, 7);
	USART1->CR1 |=(0X1<<13);
}

void USART1_SendByte(uint8_t byte)
{
	while(!(USART1->SR & (1<<6)));
	USART1->DR = byte;
}

void USART1_SendString(u8 * str)
{
	while(*str!='\0')
	{
		USART1_SendByte(*str);
		str++;
	}
}

/**
  * @brief  printf重定向到USART1。
  */
int fputc(int c, FILE* stream)
{
	USART1_SendByte(c);
	while((USART1->SR & (1<<7))==0);
	return c;
}

uint8_t  USART1_RecvData[256]="\0";  /* USART1接收缓存 */
uint32_t USART1_RecvLen=0;            /* 当前接收长度 */
uint8_t  USART1_Flag=0;               /* 接收完成标志 */
uint8_t  tmp=0;                       /* 清中断标志用临时变量 */

/**
  * @brief  USART1中断服务函数，接收数据并处理IDLE空闲中断。
  */
void USART1_IRQHandler(void)
{
	if(USART1->SR &(1<<5))
	{
		USART1_RecvData[USART1_RecvLen++] = USART1->DR;
		if(USART1_RecvLen>=256)
		{
			USART1_RecvLen=0;
		}
	}
	if(USART1->SR &(1<<4))
	{
		tmp=USART1->SR;
		tmp=USART1->DR;

		if(memcmp(USART1_RecvData,"event",5)!=NULL)
		{

		}

		memset(USART1_RecvData,0,256);
		USART1_RecvLen=0;
	}
}

/**
  * @brief  处理调试串口命令，当前保留LED调试命令入口。
  */
void USART1_Recv(void)
{
	if(USART1_Flag==1)
	{
		if(strstr((char*)USART1_RecvData,"led_breath")!=NULL)
		{
			printf("LED_Breath\r\n");
		}
		if(strstr((char*)USART1_RecvData,"led_water")!=NULL)
		{
			printf("LED_Water\r\n");
		}
		USART1_RecvLen=0;
		memset(USART1_RecvData,0,sizeof(USART1_RecvData));
	}
	USART1_Flag=0;
}
