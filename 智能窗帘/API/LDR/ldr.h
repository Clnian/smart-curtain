#ifndef _LDR_H_
#define _LDR_H_

#include "stm32f10x.h"
#include <math.h>

/* ==================== 光照传感器接口 ==================== */
void LDR_Init(void);                       /* 初始化PA4/ADC1通道4 */
uint16_t LDR_ADC_Value(uint8_t channel);   /* 读取指定ADC通道原始值 */

extern uint16_t ldr;                       /* 当前光照强度，单位Lux */

void GET_LDR(void);                        /* 周期采集并更新ldr */

#endif
