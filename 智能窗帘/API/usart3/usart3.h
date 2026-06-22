#ifndef	_USART3_H_
#define	_USART3_H_

#include "stm32f10x.h"
#include "stdio.h"
#include "string.h"

/**
  * @note  文件名沿用usart3，但实际配置的是USART1（PA9/PA10）。
  *        当前工程电机也使用PA9/PA10，提交前如需串口调试，应避免与电机同时启用。
  */
void USART1_Config(uint32_t BRR);   /* USART1初始化 */
void USART1_SendByte(uint8_t byte); /* 发送一个字节 */
void USART1_Recv(void);             /* 处理接收命令 */
void USART1_SendString(u8 * str);   /* 发送字符串 */

#endif
