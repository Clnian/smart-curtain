/**
  ******************************************************************************
  * @file    main.c
  * @brief   智能窗帘光照与温度联动控制系统 - 主程序
  *
  * 硬件平台：STM32F103C8T6
  * 功能模块：
  *   - OLED显示（I2C, PB8/PB9）
  *   - DHT11温湿度采集（PA6，单总线）
  *   - 光敏电阻光照采集（PA4，ADC）
  *   - 28BYJ-48步进电机窗帘控制（PA8~PA11）
  *   - LED报警指示灯（PB5~PB7）
  *   - ESP8266-01S WiFi + OneNET MQTT云平台（USART2, PA2/PA3）
  *
  * 主循环采用 uint16_t 计数器方式调度任务，非阻塞执行
  * SysTick 1ms中断中执行：Key_Check() + Motor_Task()
  ******************************************************************************
  */

#include "stm32f10x.h"
#include "Delay.h"
#include "usart3.h"
#include "OLED.h"
#include "systick.h"
#include "dht11.h"
#include "math.h"
#include "ledb.h"
#include "esp8266.h"
#include "ldr.h"
#include "motor.h"
#include "key.h"

/* ==================== 任务调度计数器 ==================== */
uint64_t Key_Time;   /* 按键+OLED刷新时间戳，周期100ms */
uint64_t Tem_Time;   /* 温湿度采集时间戳，周期500ms */
uint64_t Ldr_Time;   /* 光照采集时间戳，周期500ms */
uint64_t Warn_Time;  /* 报警逻辑+电机控制时间戳，周期200ms */
uint64_t Wifi_Time;  /* WiFi数据上传时间戳，周期200ms */

/* 以下变量为保留变量（当前未使用） */
uint8_t  win;        /* 窗户状态 */
uint8_t  shi;        /* 湿度状态 */
u16      pm;         /* PM2.5数据 */
uint16_t CO2_Time;   /* CO2采集计数器 */

/**
  * @brief  主函数
  * 作者：程磊
  * 时间：2026-04-27 16:35:27 +08:00
  * @note   初始化顺序：SysTick → OLED → USART1 → LED → 按键 → 光敏
  *         → 电机 → DHT11 → ESP8266 USART2 → WiFi+MQTT连接
  *
  *         主循环任务调度（基于SysCount毫秒时间戳，非阻塞）：
  *         - 100ms: 按键扫描 + OLED界面刷新
  *         - 200ms: WiFi处理（下行优先 + 轮询上报1个属性）
  *         - 200ms: 报警逻辑判断 + 电机联动控制
  *         - 500ms: DHT11温湿度采集
  *         - 500ms: 光敏电阻光照采集
  */
int main()
{
    /* 系统时基初始化：SysTick 1ms中断 */
    SysTick_Config(72000);  /* 72MHz / 72000 = 1ms */
    
    /* 外设初始化 */
    OLED_Init();            /* OLED显示屏初始化（I2C, PB8/PB9） */
    USART1_Config(115200);  /* USART1调试串口初始化（PA9/PA10） */
    led_Init();             /* LED指示灯初始化（PB5~PB7） */
    Key_Init();             /* 按键初始化（KEY1=PC13, KEY2=PC14, KEY3=PC15, KEY4=PA0） */
    LDR_Init();             /* 光敏传感器ADC初始化（PA4, ADC1通道4） */
    Motor_Init();           /* 步进电机初始化（PA8~PA11, ULN2003驱动） */
    DHT11_Init();           /* DHT11温湿度传感器初始化（PA6） */

    /* WiFi + MQTT初始化 */
    ESP8266_Config(115200); /* USART2初始化（PA2=TX, PA3=RX） */
    ESP_01S_Config();       /* ESP8266 WiFi连接 + OneNET MQTT配置 */

    /* ==================== 主循环 ==================== */
    while(1) {
        uint64_t now = SysCount;

        /* 按键扫描 + OLED界面刷新，每100ms执行一次 */
        if(now - Key_Time >= 100) { Key_Time = now; Action_Key_Process(); }
        
        /* DHT11温湿度采集，每500ms执行一次 */
        if(now - Tem_Time >= 500) { Tem_Time = now; Get_Tem_Hum(); }
        
        /* 光敏电阻光照采集，每500ms执行一次 */
        if(now - Ldr_Time >= 500) { Ldr_Time = now; GET_LDR(); }
        
        /* 报警逻辑 + 电机联动控制，每200ms执行一次 */
        if(now - Warn_Time >= 200) { Warn_Time = now; alarm_int(); }
        
        /* WiFi处理：先处理下行，再轮询上报属性，避免ESP8266被连续AT命令占满 */
        if(now - Wifi_Time >= 200) { Wifi_Time = now; Wifi_DataUp(); }
    }
    return 0;
}
