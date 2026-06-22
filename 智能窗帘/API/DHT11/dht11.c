/**
  ******************************************************************************
  * @file    dht11.c
  * @brief   DHT11温湿度传感器驱动，负责单总线时序、数据校验和全局温湿度更新。
  * 作者：程磊
  * 时间：2026-04-27 16:35:27 +08:00
  ******************************************************************************
  */

#include "dht11.h"
#include "Delay.h"

/**
  * @brief  读取并显示DHT11温湿度，主要用于独立调试。
  */
void DHT11_Display_Data(void)
{
	OLED_Printf(28,0,OLED_8X16,"Smart Curtain");
	DHT11_Read_Data(temp,humt);
	Real_Hum=humt[0]+((float)humt[1]/10);
	Real_Tem=temp[0]+((float)temp[1]/10);
	OLED_Printf(0,16, OLED_8X16, "Tem:%.1fC",(float)Real_Tem);
	OLED_Printf(0,32, OLED_8X16, "Hum:%.1f%%",(float)Real_Hum);
	OLED_Update();
}

/**
  * @brief  发送DHT11起始信号。
  * @note   主机拉低DQ约20ms后释放总线，等待传感器响应。
  */
void DHT11_Rst(void)
{
	DHT11_Mode(OUT);   /* 切换为输出模式 */
	DHT11_Low;         /* 拉低DQ */
	Delay_ms(20);      /* 起始低电平保持20ms */
	DHT11_High;        /* 释放DQ */
	Delay_us(30);      /* 等待DHT11响应窗口 */
}

/**
  * @brief  检测DHT11响应。
  * @retval 0=检测到响应，1=超时未响应。
  */
uint8_t DHT11_Check(void)
{
	uint8_t retry=0;
	DHT11_Mode(IN);  /* 切换为输入模式 */
    while (GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN)&&retry<100) /* 等待DHT11拉低 */
	{
		retry++;
		Delay_us(1);
	};
	if(retry>=100)return 1;  /* 等待低电平超时 */
	else retry=0;
    while (!GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN)&&retry<100) /* 等待DHT11拉高 */
	{
		retry++;
		Delay_us(1);
	};
	if(retry>=100)return 1;  /* 等待高电平超时 */
	return 0;
}

/**
  * @brief  读取DHT11一位数据。
  * @retval 0或1。
  * @note   低电平结束后延时40us判断，高电平仍存在则为1，否则为0。
  */
uint8_t DHT11_Read_Bit(void)
{
 	uint8_t retry=0;
	while(GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN)&&retry<100) /* 等待总线变低 */
	{
		retry++;
		Delay_us(1);
	}
	retry=0;
	while(!GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN)&&retry<100) /* 等待数据高电平开始 */
	{
		retry++;
		Delay_us(1);
	}
	Delay_us(40);  /* 在40us处采样电平 */
	if(GPIO_ReadInputDataBit(DHT11_GPIO_PORT,DHT11_GPIO_PIN))return 1;
	else return 0;
}

/**
  * @brief  读取DHT11一个字节，高位先出。
  */
uint8_t DHT11_Read_Byte(void)
{
	uint8_t i,dat;
	dat=0;
	for (i=0;i<8;i++)
	{
		dat<<=1;
		dat|=DHT11_Read_Bit();
	}
	return dat;
}

/**
  * @brief  读取DHT11温湿度数据。
  * @param  temp 温度数据缓存：[0]整数，[1]小数。
  * @param  humi 湿度数据缓存：[0]整数，[1]小数。
  * @retval 0=读取成功，1=传感器无响应。
  */
uint8_t DHT11_Read_Data(uint8_t *temp,uint8_t *humi)
{
 	uint8_t buff[5];
	uint8_t i;
	DHT11_Rst();
	int t=DHT11_Check();
	if(t==0)
	{
		for(i=0;i<5;i++)
		{
			buff[i]=DHT11_Read_Byte();
		}
		if((buff[0]+buff[1]+buff[2]+buff[3])==buff[4])  /* 校验和正确才更新数据 */
		{
			*humi=buff[0];
			*temp=buff[2];
		}
	}
	else return 1;
	return 0;
}

/**
  * @brief  初始化DHT11 GPIO，并检测传感器是否在线。
  * @retval 0=在线，1=未响应。
  */
uint8_t DHT11_Init(void)
{
	RCC_APB2PeriphClockCmd(DHT11_GPIO_CLK, ENABLE);
 	GPIO_InitTypeDef  GPIO_InitStructure;

 	GPIO_InitStructure.GPIO_Pin = DHT11_GPIO_PIN;
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
 	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStructure);
 	GPIO_SetBits(DHT11_GPIO_PORT,DHT11_GPIO_PIN);

	DHT11_Rst();
	return DHT11_Check();
}

/**
  * @brief  切换DHT11单总线方向。
  * @param  mode OUT=主机输出，IN=主机输入。
  */
void DHT11_Mode(uint8_t mode)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	if(mode)
	{
		GPIO_InitStructure.GPIO_Pin = DHT11_GPIO_PIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	}
	else
	{
		GPIO_InitStructure.GPIO_Pin =  DHT11_GPIO_PIN;
		GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	}
	GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStructure);
}

uint8_t humt[4]={0};   /* 湿度原始数据 */
uint8_t temp[4]={0};   /* 温度原始数据 */
float Real_Hum;        /* 实际湿度 */
float Real_Tem;        /* 实际温度 */

/**
  * @brief  周期读取温湿度，并更新Real_Hum和Real_Tem。
  */
void Get_Tem_Hum(void)
{
	DHT11_Read_Data(temp,humt);
	Real_Hum=humt[0]+((float)humt[1]/10);
	Real_Tem=temp[0]+((float)temp[1]/10);
}
