#ifndef _DELAY_H_
#define _DELAY_H_

#include "stm32f10x.h"

/* 毫秒延时宏：由微秒延时函数换算得到。 */
#define Delay_ms(x) Delay_us(1000*x)

void Delay_us(uint32_t time);  /* 微秒级阻塞延时 */

#endif
