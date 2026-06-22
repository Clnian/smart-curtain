#ifndef _ESP8266_H_
#define _ESP8266_H_

#include "stm32f10x.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"
#include "delay.h"
#include "DHT11.h"
#include "key.h"
#include "ledb.h"
#include "stdbool.h"
#include "usart3.h"
#include "ldr.h"
#include "motor.h"

/* ==================== ESP8266接收数据结构体 ==================== */
typedef struct ESPDATA{
	uint16_t len;          /* 接收缓冲区数据长度 */
	char buf[512];         /* 接收缓冲区（512字节防止9条MQTTPUB回应+MQTTSUBRECV同时溢出） */
	uint8_t flag;          /* 一帧接收完成标志：1=完成, 0=未完成 */
	uint8_t connect_flag;  /* MQTT连接成功标志：1=已连接, 0=未连接 */
	uint8_t Err;           /* AT初始化错误码：1=RST 2=ATE0 3=CWMODE 4=CWJAP 6=USERCFG 7=CONN 8=SUB */
	bool Mode_Real_Hum;    /* 湿度超限报警标志 */
	bool Mode_Real_Tem;    /* 温度超限报警标志 */
	bool Mode_ldr;         /* 光照超限报警标志 */
}_ESPDATA;

/* ==================== WiFi连接状态定义 ==================== */
#define WIFI_FALSE 		0   /* WiFi/MQTT断开 */
#define WIFI_OK			1   /* WiFi/MQTT连接成功 */

/* ==================== WiFi账号密码 ==================== */
#define AP    		("cl")         /* WiFi名称 */
#define AP_PWD    	("12345678")   /* WiFi密码 */

/* ==================== OneNET MQTT连接参数 ==================== */
#define ClientId	("home")        /* 设备名 */
#define username	("BTpvEtwX1a")  /* 产品ID */
#define password	("version=2018-10-31&res=products%2FBTpvEtwX1a%2Fdevices%2Fhome&et=1832067109&method=md5&sign=CxUhJ6FqWg2M2t9orRz1zA%3D%3D")  /* MQTT鉴权token */

#define SERVER_IP	("mqtts.heclouds.com")  /* OneNET MQTT服务器地址 */
#define SERVER_PORT (1883)                  /* MQTT端口号 */

/* ==================== MQTT主题 ==================== */
#define Pub_Topic	("$sys/BTpvEtwX1a/home/thing/property/post")           /* 设备属性上报主题 */
#define SUBSCRIBE_TOPIC "$sys/BTpvEtwX1a/home/thing/property/set"          /* 设备属性设置主题（下行命令） */
#define SET_REPLY_TOPIC "$sys/BTpvEtwX1a/home/thing/property/set_reply"    /* 设备属性设置回复主题 */

/* ==================== 函数声明 ==================== */
void ESP8266_Config(uint32_t brr);          /* USART2硬件初始化（PA2=TX, PA3=RX） */
void ESP8266_Sendbyte(uint8_t byte);        /* 发送单字节 */
void ESP8266_SendString(char *buf, uint16_t len);  /* 阻塞发送字符串 */
uint8_t AT_Command(char* AT_Cmd, char* ack, uint16_t time, uint8_t num);  /* 发送AT指令并等待应答 */
uint8_t ESP_01S_Config(void);              /* ESP8266 WiFi+MQTT完整初始化流程 */
uint8_t WIFI_ReconnectSilent(void);        /* WiFi/MQTT后台静默重连，不刷新OLED */
void MQTT_Publish(void);                   /* MQTT通用发布（保留未使用） */
void Clear_Data_buf(void);                 /* 清空USART2接收缓冲区 */
uint8_t WIFI_GetWifiConnectionStatus(void);   /* 获取WiFi连接状态 */
uint8_t WIFI_GetMQTTConnectionStatus(void);    /* 获取OneNET MQTT连接状态 */
extern _ESPDATA Esp8266_RecvData;          /* ESP8266全局数据结构体 */

uint8_t Wifi_ParseRecv(void);              /* 解析OneNET下行指令（curtain/mode/tem_set/ldr_set），1=已处理下行 */
void Wifi_DataUp(void);                    /* 定期数据上传入口（主循环每200ms调用） */
void Tem_Hum_Publish(void);                /* 上传温度和湿度数据 */
void MQ2_Publish(void);                    /* 上传MQ2传感器数据（保留） */
void Heating_Tem(void);                    /* 上传温度报警标志 */
void Heating_Hum(void);                    /* 上传湿度报警标志 */
void Heating_MQ7(void);                    /* 上传MQ7传感器数据（保留） */
void LDR_Publish(void);                    /* 上传光照强度数据 */
void Heating_curtain(void);               /* 上传窗帘开合度数据 */
void Heating_mode(void);                   /* 上传自动/手动模式数据 */
void Tem_Set_Publish(void);               /* 上传温度阈值 */
void Ldr_Set_Publish(void);               /* 上传光照阈值 */
extern uint8_t win;                        /* 窗户状态（保留） */
extern uint8_t shi;                        /* 湿度状态（保留） */
#endif
