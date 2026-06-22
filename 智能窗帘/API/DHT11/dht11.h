#ifndef	_DHT11_H_
#define	_DHT11_H_

#include "stm32f10x.h"
#include "OLED.h"
#include "Delay.h"
#include "esp8266.h"
#include "key.h"

/* ==================== DHT11引脚定义 ==================== */
#define DHT11_GPIO_PORT  GPIOA
#define DHT11_GPIO_PIN   GPIO_Pin_6   /* DHT11单总线数据引脚：PA6 */
#define DHT11_GPIO_CLK   RCC_APB2Periph_GPIOA

/* ==================== 单总线方向定义 ==================== */
#define OUT 1  /* 主机输出模式 */
#define IN  0  /* 主机输入模式 */

/* ==================== 数据线电平控制 ==================== */
#define DHT11_Low  GPIO_ResetBits(DHT11_GPIO_PORT, DHT11_GPIO_PIN)  /* 拉低DQ */
#define DHT11_High GPIO_SetBits(DHT11_GPIO_PORT, DHT11_GPIO_PIN)    /* 拉高DQ */

/* ==================== 函数声明 ==================== */
uint8_t DHT11_Init(void);              /* 初始化DHT11并检测是否在线 */
uint8_t DHT11_Read_Data(uint8_t *temp, uint8_t *humi);  /* 读取温湿度原始数据 */
uint8_t DHT11_Read_Byte(void);         /* 读取一个字节 */
uint8_t DHT11_Read_Bit(void);          /* 读取一位数据 */
void DHT11_Mode(uint8_t mode);         /* 切换DQ方向，OUT=输出，IN=输入 */
uint8_t DHT11_Check(void);             /* 等待并检测DHT11响应 */
void DHT11_Rst(void);                  /* 发送起始信号 */
void Get_Tem_Hum(void);                /* 更新全局温湿度变量 */

/* ==================== 全局变量 ==================== */
extern uint8_t humt[4];   /* 湿度原始数据：[0]整数，[1]小数 */
extern uint8_t temp[4];   /* 温度原始数据：[0]整数，[1]小数 */
extern float Real_Hum;    /* 实际湿度值 */
extern float Real_Tem;    /* 实际温度值 */

#endif
