/**
  ******************************************************************************
  * @file    ledb.c
  * @brief   LED报警指示灯驱动，PB5~PB7低电平点亮。
  * 作者：程磊
  * 时间：2026-04-27 16:35:27 +08:00
  ******************************************************************************
  */

#include "ledb.h"

/**
  * @brief  初始化LED GPIO。
  */
void led_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7);  /* 默认全部熄灭 */

	LED3_ON;    /* 上电短闪提示 */
	LED3_OFF;
	LED4_OFF;
}

/**
  * @brief  LED3/LED4交替闪烁，用于温度或光照超限提示。
  */
void led_set(void)
{
    LED3_ON;
	Delay_ms(100);
	LED3_OFF;
    LED4_ON;
	Delay_ms(100);
	LED4_OFF;
}
