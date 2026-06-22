/**
  ******************************************************************************
  * @file    Delay.c
  * @brief   简单阻塞延时函数，供DHT11单总线时序和短延时场景使用。
  * 作者：程磊
  * 时间：2026-04-27 16:35:27 +08:00
  ******************************************************************************
  */

#include "Delay.h"
#include "core_cm3.h"

/**
  * @brief  微秒级阻塞延时。
  * @param  time 延时时间，单位us。
  * @note   通过NOP空指令粗略延时，适合短时序控制，不适合长时间等待。
  */
void Delay_us(uint32_t time)
{
	while(time--)
	{
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
	__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();__nop();
  }
}
