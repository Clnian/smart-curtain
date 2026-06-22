/**
  ******************************************************************************
  * @file    esp8266.c
  * @brief   ESP8266-01S WiFi模块 + OneNET MQTT云平台驱动
  *
  * 通信接口：USART2（PA2=TX复用推挽, PA3=RX浮空输入, 115200bps）
  * 云平台：中国移动OneNET（MQTT, mqtts.heclouds.com:1883）
  *
  * 初始化流程（ESP_01S_Config）：
  *   1. AT+RST      - 复位模块
  *   2. ATE0        - 关闭回显（减少数据量）
  *   3. AT+CWMODE=1 - Station模式
  *   4. AT+CWJAP    - 连接WiFi
  *   5. AT+MQTTUSERCFG - MQTT用户属性配置
  *   6. AT+MQTTCONN - 连接MQTT服务器
  *   7. AT+MQTTSUB  - 订阅接收主题
  *
  * 数据上传（Wifi_DataUp，主循环每200ms调用）：
  *   先处理OneNET下行指令，再按步骤轮询上报温湿度、光照、报警、
  *   窗帘位置、模式和阈值等属性。
  *   若云端链路离线，则停止普通上报，每约10秒后台静默重连一次。
  *
  * 错误处理：首次初始化失败时OLED显示错误码，KEY1=重试, KEY2=跳过进入本地运行
  ******************************************************************************
  */

#include "esp8266.h"


/*************************
 * 函数名称：ESP8266_Config
 * 函数功能：ESP8266--USART2初始化
 * 作者：程磊
 * 时间：2026-04-27 16:35:27 +08:00
 * 函数入口：brr - 波特率
 * 函数出口：无
 * 引脚说明：
 *	PA2--USART2_TX  复用推挽输出
 *	PA3--USART2_RX  浮空输入
*************************/
void ESP8266_Config(uint32_t brr)
{
	/* 使能GPIOA和USART2时钟 */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	
	/* TX: PA2复用推挽输出 */
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	/* RX: PA3浮空输入 */
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	/* USART2参数配置：115200-8-N-1 */
	USART_InitTypeDef USART_InitStruct;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  /* 无硬件流控 */
	USART_InitStruct.USART_BaudRate = brr;
	USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStruct.USART_Parity = USART_Parity_No;    /* 无校验 */
	USART_InitStruct.USART_StopBits = USART_StopBits_1;  /* 1位停止位 */
	USART_InitStruct.USART_WordLength = USART_WordLength_8b;  /* 8位数据位 */
	USART_Init(USART2, &USART_InitStruct);
	
	/* 使能接收中断和空闲中断 */
	USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);  /* 接收缓冲区非空中断 */
	USART_ITConfig(USART2, USART_IT_IDLE, ENABLE);  /* 总线空闲中断（一帧完成） */
	
	/* NVIC中断配置 */
	NVIC_InitTypeDef NVIC_InitStruct;
	NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;  /* 抢占优先级1 */
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 3;        /* 子优先级3 */
	NVIC_Init(&NVIC_InitStruct);
	
	USART_Cmd(USART2, ENABLE);  /* 使能USART2 */
}

/**
  * @brief  阻塞发送字符串到ESP8266
  * 作者：程磊
  * 时间：2026-04-27 16:35:27 +08:00
  * @param  buf: 数据缓冲区
  * @param  len: 数据长度
  * @note   等待TXE发送寄存器为空后逐字节发送，最后等待TC发送完成
  */
void ESP8266_SendString(char *buf, uint16_t len)
{
	for(int i=0; i<len; i++)
	{
		while(USART_GetFlagStatus(USART2, USART_FLAG_TXE)==RESET);  /* 等待发送寄存器为空 */
		USART_SendData(USART2, buf[i]);
	}
	while(!(USART_GetFlagStatus(USART2, USART_FLAG_TC)));  /* 等待发送完成 */
}

/* ESP8266全局数据结构体 */
_ESPDATA Esp8266_RecvData = {0};

/* Wifi_DataUp()当前由主循环每200ms调用，50次约等于10秒 */
#define WIFI_RECONNECT_INTERVAL_TICKS 50

/**
  * @brief  USART2中断服务函数（ESP8266数据接收）
  * 作者：程磊
  * 时间：2026-04-27 16:35:27 +08:00
  * @note   RXNE中断：逐字节存入缓冲区，满256字节时溢出清零
  *         IDLE中断：一帧数据接收完成，置位flag标志
  */
void USART2_IRQHandler(void)
{
	if(USART_GetITStatus(USART2, USART_IT_RXNE)==SET)  /* RXNE中断：收到一个字节 */
	{
		Esp8266_RecvData.buf[Esp8266_RecvData.len++] = USART_ReceiveData(USART2);
		if(Esp8266_RecvData.len>=512)  /* 防溢出操作 */
			Esp8266_RecvData.len=0;
		
		USART_ClearITPendingBit(USART2, USART_IT_RXNE);  /* 清除RXNE标志 */
	}
	if(USART_GetITStatus(USART2, USART_IT_IDLE)==SET)  /* IDLE空闲中断：一帧完成 */
	{
		uint8_t tmp=USART2->SR;  /* 读SR和DR清除IDLE标志（STM32硬件要求） */
		tmp=USART2->DR;
		Esp8266_RecvData.flag=1;  /* 置位接收完成标志 */
	}
}

/*************************
 * 函数名称：AT_Command
 * 函数功能：发送AT指令并等待应答
 * 作者：程磊
 * 时间：2026-04-27 16:35:27 +08:00
 * 函数入口：
 *	AT_Cmd: 要发送的AT命令
 *	ack: 要检测的回复信息关键字
 *	time: 发送完成后等待的时间(ms)
 *	num: 命令重发的次数
 * 函数出口：1: 发送成功  0: 发送失败达到最大次数
*************************/
uint8_t AT_Command(char* AT_Cmd, char* ack, uint16_t time, uint8_t num)
{
	/* 清空接收缓冲区 */
	Esp8266_RecvData.len=0;
	Esp8266_RecvData.flag=0;
	memset(Esp8266_RecvData.buf, 0, sizeof(Esp8266_RecvData.buf));
	
	while(num--)
	{	
		ESP8266_SendString(AT_Cmd, strlen(AT_Cmd));  /* 发送AT指令 */
		Delay_ms(time);  /* 等待响应 */
		
		if(Esp8266_RecvData.flag==1)  /* 收到响应 */
		{
			if(strstr(Esp8266_RecvData.buf, ack)!=NULL)  /* 响应中包含期望关键字 */
			{
				return 1;  /* 发送成功 */
			}
		}
	}
	return 0;  /* 重试次数用完，发送失败 */
}


/**
  * @brief  ESP8266 WiFi + MQTT完整初始化流程
  * 作者：程磊
  * 时间：2026-04-27 16:35:27 +08:00
  * @retval 0: 初始化完成
  * @note   初始化流程：
  *   AT+RST → ATE0 → AT+CWMODE=1 → AT+CWJAP
  *   → AT+MQTTUSERCFG → AT+MQTTCONN → AT+MQTTSUB
  *   失败时OLED显示错误码，KEY1=重试，KEY2=跳过继续执行
  *   错误码：1=RST 2=ATE0 3=CWMODE 4=CWJAP 6=USERCFG 7=CONN 8=SUB
  */
uint8_t ESP_01S_Config(void)
{
	uint8_t len=0;
	uint8_t Key_Value;
	uint8_t mqtt_status;
	uint8_t init_ok;
	char buf[256];
	
Retry:
	len++;
	init_ok = 1;
	Esp8266_RecvData.connect_flag = WIFI_FALSE;
	Esp8266_RecvData.Err = 0;
	OLED_Clear(); OLED_Update();
	OLED_Printf(25,0,OLED_8X16,"WIFI_Init...");
	OLED_Update();
	
	/* 第1步：AT重启指令 */
	if(AT_Command("AT+RST\r\n", "", 200, 5)==0)
	{
		init_ok = 0;
		Esp8266_RecvData.connect_flag = WIFI_FALSE;
		if(Esp8266_RecvData.Err==0)		
			Esp8266_RecvData.Err=1;  /* 错误码1：RST失败 */
	}
	else
	{
		OLED_Printf(0,16,OLED_8X16,"AT+RST"); OLED_Update();
	}
	
	/* 第2步：关闭回显（减少数据量） */
	if(AT_Command("ATE0\r\n", "OK", 200, 5)==0)
	{
		init_ok = 0;
		Esp8266_RecvData.connect_flag = WIFI_FALSE;
		if(Esp8266_RecvData.Err==0)		
			Esp8266_RecvData.Err=2;  /* 错误码2：ATE0失败 */
	}
	else
	{
		OLED_Printf(0,32,OLED_8X16,"ATE0"); OLED_Update();
	}	
	
	/* 第3步：设置Station模式 */
	if(AT_Command("AT+CWMODE=1\r\n", "OK", 250, 5)==0)
	{
		init_ok = 0;
		Esp8266_RecvData.connect_flag = WIFI_FALSE;
		if(Esp8266_RecvData.Err==0)		
			Esp8266_RecvData.Err=3;  /* 错误码3：CWMODE失败 */
	}
	else
	{
		OLED_Printf(0,48,OLED_8X16,"AT+CWMODE=1"); OLED_Update();
	}
	
	/* 第4步：连接WiFi */
	sprintf(buf, "AT+CWJAP=\"%s\",\"%s\"\r\n", AP, AP_PWD);

	if(AT_Command(buf, "WIFI CONNECTED", 5000, 5)==0)
	{
		init_ok = 0;
		Esp8266_RecvData.connect_flag = WIFI_FALSE;
		if(Esp8266_RecvData.Err==0)		
			Esp8266_RecvData.Err=4;  /* 错误码4：CWJAP失败 */
	}
	else
	{	
		OLED_Clear(); OLED_Update();
		OLED_Printf(0,0,OLED_8X16,"WIFI CONNECTED"); OLED_Update();
		memset(buf, 0, 256);
	}
	
	/* 第5步：设置MQTT用户属性（产品ID、设备名、鉴权token） */
	sprintf(buf, "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n", ClientId, username, password);
	if(AT_Command(buf, "OK", 2000, 5)==0) 
	{
		init_ok = 0;
		Esp8266_RecvData.connect_flag = WIFI_FALSE;
		if(Esp8266_RecvData.Err==0)		
			Esp8266_RecvData.Err=6;  /* 错误码6：MQTTUSERCFG失败 */
	}
	else
	{
		OLED_Printf(0,32,OLED_8X16,"AT+MQTTUSERCFG=0"); OLED_Update();
		memset(buf, 0, 256);
	}
	
	Delay_ms(500);
	
	/* 第6步：连接MQTT服务器 */
	sprintf(buf, "AT+MQTTCONN=0,\"%s\",%d,1\r\n", SERVER_IP, SERVER_PORT);
	if(AT_Command(buf, "OK", 500, 5)==0)
	{
		init_ok = 0;
		Esp8266_RecvData.connect_flag = WIFI_FALSE;
		if(Esp8266_RecvData.Err==0)		
			Esp8266_RecvData.Err=7;  /* 错误码7：MQTTCONN失败 */
	}
	else
	{
		OLED_Printf(0,48,OLED_8X16,"AT+MQTTCONN=0"); OLED_Update();
	}
	
	/* 第7步：订阅接收主题 */
	sprintf(buf, "AT+MQTTSUB=0,\"%s\",0\r\n", SUBSCRIBE_TOPIC);
	if(AT_Command(buf, "OK", 800, 5)==0)
	{
		init_ok = 0;
		Esp8266_RecvData.connect_flag = WIFI_FALSE;
		if(Esp8266_RecvData.Err==0)
			Esp8266_RecvData.Err=8;  /* 错误码8：MQTTSUB失败 */
	}
	else
	{
		/* MQTT连接和订阅都成功后，才标记云端链路可用 */
		Delay_ms(500);
		mqtt_status = WIFI_GetMQTTConnectionStatus();
		if(init_ok == 0 || mqtt_status == WIFI_FALSE)
		{
			Esp8266_RecvData.connect_flag = WIFI_FALSE;
			if(Esp8266_RecvData.Err==0)
				Esp8266_RecvData.Err=7;  /* MQTT已断开 */
		}
		else
		{
			Esp8266_RecvData.connect_flag = WIFI_OK;
			OLED_Clear();
			OLED_Printf(0,0,OLED_8X16,"AT+MQTTSUB=0"); OLED_Update();
			OLED_Printf(0,16,OLED_8X16,"WIFI_OK!!!"); OLED_Update();
		}
		Delay_ms(2500);
		Clear_Data_buf();
		OLED_Select(1);  /* 切换OLED到主页 */
	}	
	
	
	/* 如果连接失败，显示错误信息并等待用户操作 */
	if(Esp8266_RecvData.connect_flag != WIFI_OK)
	{
		OLED_Clear();
		OLED_Printf(0,0,OLED_8X16,"WIFI_Faild...");
		OLED_Printf(0,16,OLED_8X16,"WIFI_Err:%d...", Esp8266_RecvData.Err);  /* 显示错误标志 */
		Clear_Data_buf();
		OLED_Update();
		while(1)
        {
            Key_Value = Key_Scan();
            if(Key_Value == KEY2_PRESS)  /* KEY2：跳过，继续执行 */
            {
				break;
            }
            else if(Key_Value == KEY1_PRESS)  /* KEY1：重新初始化 */
            {
                goto Retry;
            }
        }
	}
	return 0;
}

/**
  * @brief  清空USART2接收缓冲区
  */
void Clear_Data_buf(void)
{
	memset(Esp8266_RecvData.buf, 0, sizeof(Esp8266_RecvData.buf));
	Esp8266_RecvData.len=0;
}

/**
  * @brief  MQTT通用发布函数（保留未使用）
  * @note   上传温度和湿度的旧版格式，已被Tem_Hum_Publish替代
  */
void MQTT_Publish(void)
{
	char str[256]="\0";
	sprintf(str, "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"wendu\\\":{\\\"value\\\":%f}\\,\\\"sshidu\\\":{\\\"value\\\":%f}}}\",0,0\r\n",
	Pub_Topic, Real_Tem, Real_Hum);
	
	printf("%s", str);
	
	ESP8266_SendString(str, strlen(str));
}


/* ---- WIFI_GetWifiConnectionStatus ---- */
/**
  * @brief  获取WiFi连接状态
  * @retval WIFI_FALSE: 断开, WIFI_OK: 已连接, 0xFF: 未知
  */
uint8_t WIFI_GetWifiConnectionStatus(void)
{
    if(strstr((char*)Esp8266_RecvData.buf, "WIFI DISCONNECT") != NULL) return WIFI_FALSE;
    if(strstr((char*)Esp8266_RecvData.buf, "WIFI GOT IP") != NULL) return WIFI_OK;
    return 0xFF;
}


/* ---- WIFI_GetMQTTConnectionStatus ---- */
/**
  * @brief  获取OneNET MQTT连接状态
  * @retval WIFI_FALSE: 断开, WIFI_OK: 已连接, 0xFF: 未知
  */
uint8_t WIFI_GetMQTTConnectionStatus(void)
{
    if(strstr((char*)Esp8266_RecvData.buf, "+MQTTDISCONNECTED:0") != NULL) return WIFI_FALSE;
    if(strstr((char*)Esp8266_RecvData.buf, "+MQTTCONNECTED:0") != NULL) return WIFI_OK;
    return 0xFF;
}


/**
  * @brief  WiFi/MQTT后台静默重连
  * 作者：程磊
  * 时间：2026-04-27 16:35:27 +08:00
  * @note   不刷新OLED，不停在错误页面；成功后恢复云端上报，失败保持本地离线运行。
  * @retval WIFI_OK: 重连成功, WIFI_FALSE: 重连失败
  */
uint8_t WIFI_ReconnectSilent(void)
{
	char buf[256];
	uint8_t mqtt_status;

	Esp8266_RecvData.connect_flag = WIFI_FALSE;

	if(AT_Command("ATE0\r\n", "OK", 200, 1)==0)
		return WIFI_FALSE;

	if(AT_Command("AT+CWMODE=1\r\n", "OK", 250, 1)==0)
		return WIFI_FALSE;

	sprintf(buf, "AT+CWJAP=\"%s\",\"%s\"\r\n", AP, AP_PWD);
	if(AT_Command(buf, "OK", 5000, 1)==0)
		return WIFI_FALSE;

	sprintf(buf, "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n", ClientId, username, password);
	if(AT_Command(buf, "OK", 2000, 1)==0)
		return WIFI_FALSE;

	sprintf(buf, "AT+MQTTCONN=0,\"%s\",%d,1\r\n", SERVER_IP, SERVER_PORT);
	if(AT_Command(buf, "OK", 800, 1)==0)
		return WIFI_FALSE;

	sprintf(buf, "AT+MQTTSUB=0,\"%s\",0\r\n", SUBSCRIBE_TOPIC);
	if(AT_Command(buf, "OK", 800, 1)==0)
		return WIFI_FALSE;

	Delay_ms(300);
	mqtt_status = WIFI_GetMQTTConnectionStatus();
	if(mqtt_status == WIFI_FALSE)
	{
		Esp8266_RecvData.connect_flag = WIFI_FALSE;
		return WIFI_FALSE;
	}

	Esp8266_RecvData.connect_flag = WIFI_OK;
	Clear_Data_buf();
	return WIFI_OK;
}



/* ---- Tem_Hum_Publish ---- */
/**
  * @brief  上传温度和湿度数据到OneNET云平台
  * @note   Thing model格式：{"id":"123","params":{"Tmp":{"value":xx.x},"Hum":{"value":xx.x}}}
  *         供主循环每200ms调用（Wifi_DataUp中）
  */
void Tem_Hum_Publish(void)
{
	char str[256]="\0";
	sprintf(str, "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"Tmp\\\":{\\\"value\\\":%.1f}\\,\\\"Hum\\\":{\\\"value\\\":%.1f}}}\",0,0\r\n",
	Pub_Topic, Real_Tem, Real_Hum);
	
	ESP8266_SendString(str, strlen(str));
}


/* ---- LDR_Publish ---- */
/**
  * @brief  上传光照强度数据到OneNET云平台
  * @note   Thing model格式：{"id":"123","params":{"ldr":{"value":xxx}}}
  */
void LDR_Publish(void)
{
	char str[256]="\0";
	sprintf(str, "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"ldr\\\":{\\\"value\\\":%3d}}}\",0,0\r\n",
	Pub_Topic, ldr);
	
	ESP8266_SendString(str, strlen(str));
}


/* ---- Heating_Tem ---- */
/**
  * @brief  上传温度报警标志
  * @note   当Real_Tem > Tem_Up时 Mode_Real_Tem=true
  */
void Heating_Tem(void)
{
	char str[256]="\0";
	sprintf(str, "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"Tem_beep\\\":{\\\"value\\\":%s}}}\",0,0\r\n",
	Pub_Topic, Esp8266_RecvData.Mode_Real_Tem ? "true":"false");
	
	ESP8266_SendString(str, strlen(str));
}


/* ---- Heating_Hum ---- */
/**
  * @brief  上传湿度报警标志
  * @note   当Real_Hum超限时 Mode_Real_Hum=true
  */
void Heating_Hum(void)
{
	char str[256]="\0";
	sprintf(str, "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"Hum_beep\\\":{\\\"value\\\":%s}}}\",0,0\r\n",
	Pub_Topic, Esp8266_RecvData.Mode_Real_Hum ? "true":"false");
	
	ESP8266_SendString(str, strlen(str));
}


/* ---- Heating_JW01 (CO2_beep) ---- */
/**
  * @brief  上传光照报警标志（CO2_beep字段名保留）
  * @note   当ldr > ldr_Down时 Mode_ldr=true
  */
void Heating_JW01(void)
{
	char str[256]="\0";
	sprintf(str, "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"CO2_beep\\\":{\\\"value\\\":%s}}}\",0,0\r\n",
	Pub_Topic, Esp8266_RecvData.Mode_ldr ? "true":"false");
	
	ESP8266_SendString(str, strlen(str));
}


/* ---- Heating_curtain ---- */
/**
  * @brief  上传窗帘开合度数据（0~100%）
  */
void Heating_curtain(void)
{
	char str[256]="\0";
	sprintf(str, "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"curtain\\\":{\\\"value\\\":%d}}}\",0,0\r\n",
	Pub_Topic, Curtain_Position);
	
	ESP8266_SendString(str, strlen(str));
}


/* ---- Heating_mode ---- */
/**
  * @brief  上传自动/手动模式数据
  * @note   Auto_Mode: 1=自动, 0=手动
  */
void Heating_mode(void)
{
	char str[256]="\0";
	sprintf(str, "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"mode\\\":{\\\"value\\\":%d}}}\",0,0\r\n",
	Pub_Topic, Auto_Mode);
	
	ESP8266_SendString(str, strlen(str));
}



/* ---- Tem_Set_Publish ---- */
/**
  * @brief  上传温度阈值到OneNET（供小程序 setting 页读取）
  */
void Tem_Set_Publish(void)
{
	char str[256] = "\0";
	sprintf(str, "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"tem_set\\\":{\\\"value\\\":%.1f}}}\",0,0\r\n",
	Pub_Topic, Tem_Up);
	ESP8266_SendString(str, strlen(str));
}


/* ---- Ldr_Set_Publish ---- */
/**
  * @brief  上传光照阈值到OneNET（供小程序 setting 页读取）
  */
void Ldr_Set_Publish(void)
{
	char str[256] = "\0";
	sprintf(str, "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":\\\"123\\\"\\,\\\"params\\\":{\\\"ldr_set\\\":{\\\"value\\\":%d}}}\",0,0\r\n",
	Pub_Topic, ldr_Down);
	ESP8266_SendString(str, strlen(str));
}




/* ---- Wifi_ParseRecv ---- */
/**
  * @brief   ESP8266 缓冲区中 OneNET 推送的下行指令
  * 作者：程磊
  * 时间：2026-04-27 16:35:27 +08:00
  * @note   OneNET 收到小程序 setProperty 请求后，通过 MQTT 推送：
  *           +MQTTSUBRECV:0,"topic",len,{"id":"x","params":{"curtain":{"value":50}}}
  *         本函数每次 Wifi_DataUp() 开头调用，解析后清空缓冲区。
  *         支持字段：
  *           curtain  → Motor_MoveTo()     控制窗帘位置 0~100
  *           mode     → Auto_Mode          切换自动/手动
  *           tem_set  → Tem_Up             更新温度阈值 -20~60℃
  *           ldr_set  → ldr_Down           更新光照阈值 0~1000 lx
  */
uint8_t Wifi_ParseRecv(void)
{
	if (Esp8266_RecvData.len == 0 || Esp8266_RecvData.flag == 0) return 0;

	if (strstr(Esp8266_RecvData.buf, "+MQTTDISCONNECTED:0") != NULL) {
		Esp8266_RecvData.connect_flag = WIFI_FALSE;
	} else if (strstr(Esp8266_RecvData.buf, "+MQTTCONNECTED:0") != NULL) {
		Esp8266_RecvData.connect_flag = WIFI_OK;
	}

	/* 查找下行订阅消息标记 */
	char *p = strstr(Esp8266_RecvData.buf, "+MQTTSUBRECV:");
	if (!p) goto done;

	/* 定位 JSON 载荷（第一个 '{' ） */
	char *json = strchr(p, '{');
	if (!json) goto done;

	/* 提取请求 id（用于回复 OneNET） */
	char msg_id[64] = "1";
	char *id_p = strstr(json, "\"id\":");
	if (id_p) {
		char *id_start = strchr(id_p + 5, '\"');
		if (id_start) {
			id_start++;
			char *id_end = strchr(id_start, '\"');
			if (id_end) {
				int len = id_end - id_start;
				if (len > 0 && len < 63) {
					memcpy(msg_id, id_start, len);
					msg_id[len] = '\0';
				}
			}
		}
	}

	/*  curtain（窗帘开合度 0~100） */
	char *curtain_p = strstr(json, "\"curtain\"");
	if (curtain_p) {
		char *val_p = strstr(curtain_p, "\"value\":");
		char *direct_p = strchr(curtain_p, ':');
		if (val_p || direct_p) {
			int v = val_p ? atoi(val_p + 8) : atoi(direct_p + 1);
			if (v >= 0 && v <= 100) Motor_MoveTo((uint8_t)v);
		}
	}

	/*  mode（1=自动，0=手动） */
	char *mode_p = strstr(json, "\"mode\"");
	if (mode_p) {
		char *val_p = strstr(mode_p, "\"value\":");
		char *direct_p = strchr(mode_p, ':');
		if (val_p || direct_p) Auto_Mode = (val_p ? atoi(val_p + 8) : atoi(direct_p + 1)) ? Auto : Hand;
	}

	/*  tem_set（温度阈值 -20~60℃） */
	char *tem_set_p = strstr(json, "\"tem_set\"");
	if (tem_set_p) {
		char *val_p = strstr(tem_set_p, "\"value\":");
		char *direct_p = strchr(tem_set_p, ':');
		if (val_p || direct_p) {
			float v = val_p ? atof(val_p + 8) : atof(direct_p + 1);
			if (v >= -20.0f && v <= 60.0f) Tem_Up = v;
		}
	}

	/*  ldr_set（光照阈值 0~1000 lx） */
	char *ldr_set_p = strstr(json, "\"ldr_set\"");
	if (ldr_set_p) {
		char *val_p = strstr(ldr_set_p, "\"value\":");
		char *direct_p = strchr(ldr_set_p, ':');
		if (val_p || direct_p) {
			int v = val_p ? atoi(val_p + 8) : atoi(direct_p + 1);
			if (v >= 0 && v <= 1000) ldr_Down = (uint16_t)v;
		}
	}

	/* 立即回复OneNET属性设置（不等上传完成，reply优先级最高） */
	/* 如果等9条上传完再回复，OneNET早就超时了（约5秒） */
	char reply_str[256] = "\0";
	sprintf(reply_str, "AT+MQTTPUB=0,\"%s\",\"{\\\"id\\\":\\\"%s\\\",\\\"code\\\":200,\\\"msg\\\":\\\"success\\\"}\",0,0\r\n",
		SET_REPLY_TOPIC, msg_id);
	Clear_Data_buf();
	Esp8266_RecvData.flag = 0;
	ESP8266_SendString(reply_str, strlen(reply_str));
	return 1;  /* 回复完毕，直接返回，避免普通上报抢占回复窗口 */

done:
	Clear_Data_buf();
	Esp8266_RecvData.flag = 0;
	return 0;
}


/**
  * @brief  定期数据上传入口（主循环每200ms调用一次）
  * 作者：程磊
  * 时间：2026-04-27 16:35:27 +08:00
  * @note   在线时先解析下行指令，再依次发送9个MQTT发布命令：
  *         Tem_Hum → LDR → Heating_Tem → Heating_Hum → Heating_JW01
  *         → Heating_curtain → Heating_mode → Tem_Set → Ldr_Set
  *         离线时不做普通上报，只按计数周期调用静默重连，避免OLED被AT流程刷屏。
  *         ESP8266-01S处理AT指令需要数十到数百毫秒，因此这里采用分步上报。
  */
void Wifi_DataUp(void)
{
	static uint8_t publish_step = 0;
	static uint8_t reconnect_tick = 0;

	if (Wifi_ParseRecv()) return;    /* 下行回复优先，避免OneNET属性设置超时 */

	if (Esp8266_RecvData.connect_flag != WIFI_OK)
	{
		publish_step = 0;
		reconnect_tick++;
		if (reconnect_tick >= WIFI_RECONNECT_INTERVAL_TICKS)
		{
			reconnect_tick = 0;
			WIFI_ReconnectSilent();
		}
		return;
	}

	reconnect_tick = 0;

	switch (publish_step)
	{
		case 0: Tem_Hum_Publish(); break;
		case 1: LDR_Publish(); break;
		case 2: Heating_Tem(); break;
		case 3: Heating_Hum(); break;
		case 4: Heating_JW01(); break;
		case 5: Heating_curtain(); break;
		case 6: Heating_mode(); break;
		case 7: Tem_Set_Publish(); break;
		case 8: Ldr_Set_Publish(); break;
		default: publish_step = 0; return;
	}

	publish_step++;
	if (publish_step >= 9) publish_step = 0;
}
