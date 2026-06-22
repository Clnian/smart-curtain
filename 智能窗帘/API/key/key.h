#ifndef _KEY_H_
#define _KEY_H_

#include "stm32f10x.h"
#include "esp8266.h"
#include "DHT11.h"
#include "OLED.h"
#include "OLED_Data.h"
#include "ledb.h"
#include "stdbool.h"
#include "ldr.h"
#include "motor.h"

/* ==================== 模式定义 ==================== */
#define Auto 1   /* 自动模式 */
#define Hand 0   /* 手动模式 */

/* ==================== 按键事件值 ==================== */
enum {
    KEY1_PRESS = 1,  /* KEY1按下，PC13 */
    KEY2_PRESS = 2,  /* KEY2按下，PC14 */
    KEY3_PRESS = 3,  /* KEY3按下，PC15 */
    KEY4_PRESS = 4   /* KEY4按下，PA0 */
};

/* ==================== 函数声明 ==================== */
void Key_Init(void);                  /* 按键GPIO初始化 */
void OLED_Select(uint8_t select);     /* OLED页面选择和刷新 */
void Key_Check(void);                 /* 按键去抖状态机，SysTick中调用 */
uint8_t Key_Scan(void);               /* 获取一次按键事件 */
void Action_Key_Process(void);        /* 按键处理和OLED刷新，主循环周期调用 */
void alarm_int(void);                 /* 自动窗帘控制逻辑 */

/* ==================== 全局变量声明 ==================== */
extern uint8_t  Key4_Turn_Down;  /* 当前OLED页：1=温湿度页，2=光照状态页，3=设置页 */
extern uint8_t  Select_Index;    /* 设置页光标：0=温度阈值，1=光照阈值 */

extern uint8_t  Cool_flag;       /* 预留标志 */
extern uint8_t  Power_flag;      /* 预留标志 */
extern uint8_t  Blower_flag;     /* 预留标志 */

extern uint8_t  Auto_Mode;       /* 当前模式：1=自动，0=手动 */
extern float    Tem_Up;          /* 温度阈值，默认30.0℃ */
extern uint16_t ldr_Down;        /* 光照阈值，默认500 lx */
extern uint16_t Ldr_Hyst;        /* 光照迟滞死区，默认100 lx */

#endif