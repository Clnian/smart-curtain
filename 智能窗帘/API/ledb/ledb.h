#ifndef _LEDB_H_
#define _LEDB_H_

#include "stm32f10x.h"
#include "Delay.h"

/* ==================== LEDÒý½Å¶¨Òå ==================== */
/* LED2~LED4½ÓPB5~PB7£¬µÍµçÆ½µãÁÁ¡£ */
//#define LED1_OFF   GPIOB->ODR |=(1<<4)
//#define LED1_ON    GPIOB->ODR &=~(1<<4)

#define LED2_OFF   GPIOB->ODR |=(1<<5)  /* LED2Ï¨Ãð */
#define LED2_ON    GPIOB->ODR &=~(1<<5) /* LED2µãÁÁ */

#define LED3_OFF   GPIOB->ODR |=(1<<6)  /* LED3Ï¨Ãð */
#define LED3_ON    GPIOB->ODR &=~(1<<6) /* LED3µãÁÁ */

#define LED4_OFF   GPIOB->ODR |=(1<<7)  /* LED4Ï¨Ãð */
#define LED4_ON    GPIOB->ODR &=~(1<<7) /* LED4µãÁÁ */

void Beep_Sound_Time(uint8_t time);  /* Ô¤Áô·äÃùÆ÷ÏìÉù½Ó¿Ú */
void BEEP_set(void);                  /* Ô¤Áô·äÃùÆ÷¿ØÖÆ½Ó¿Ú */
void led_set(void);                   /* LED±¨¾¯ÉÁË¸ */
void led_Init(void);                  /* LED GPIO³õÊ¼»¯ */
void Beep_Init(void);                 /* Ô¤Áô·äÃùÆ÷³õÊ¼»¯½Ó¿Ú */
#endif
