#ifndef _MOTOR_H_
#define _MOTOR_H_

#include "stm32f10x.h"

/* ===================== 电机硬件引脚定义 ===================== */
/* ULN2003 IN1~IN4 接 PA11/PA10/PA9/PA8，硬件已固定。 */
/* 注意：PA9/PA10 与 USART1_TX/RX 复用，调试串口和电机不可同时占用。 */
/* 28BYJ-48 为4相5线步进电机，常用减速比约1:64。 */
#define MOTOR_IN1_PORT   GPIOA
#define MOTOR_IN1_PIN    GPIO_Pin_11
#define MOTOR_IN2_PORT   GPIOA
#define MOTOR_IN2_PIN    GPIO_Pin_10
#define MOTOR_IN3_PORT   GPIOA
#define MOTOR_IN3_PIN    GPIO_Pin_9
#define MOTOR_IN4_PORT   GPIOA
#define MOTOR_IN4_PIN    GPIO_Pin_8

/* ===================== 引脚快速写入函数 ===================== */
static __inline void MOTOR_IN1(uint8_t a) { if (a) MOTOR_IN1_PORT->BSRR = MOTOR_IN1_PIN; else MOTOR_IN1_PORT->BRR = MOTOR_IN1_PIN; }
static __inline void MOTOR_IN2(uint8_t a) { if (a) MOTOR_IN2_PORT->BSRR = MOTOR_IN2_PIN; else MOTOR_IN2_PORT->BRR = MOTOR_IN2_PIN; }
static __inline void MOTOR_IN3(uint8_t a) { if (a) MOTOR_IN3_PORT->BSRR = MOTOR_IN3_PIN; else MOTOR_IN3_PORT->BRR = MOTOR_IN3_PIN; }
static __inline void MOTOR_IN4(uint8_t a) { if (a) MOTOR_IN4_PORT->BSRR = MOTOR_IN4_PIN; else MOTOR_IN4_PORT->BRR = MOTOR_IN4_PIN; }

/* ===================== 窗帘位置定义 ===================== */
#define MOTOR_STEPS_PER_REV    512   /* 电机输出轴约一圈步数 */
#define MOTOR_MAX_POSITION     100   /* 窗帘最大关闭度，100=全关 */
#define MOTOR_MIN_POSITION     0     /* 窗帘最小关闭度，0=全开 */

extern uint8_t Curtain_Position;      /* 当前窗帘关闭度：0=全开，100=全关 */

/* ===================== 函数声明 ===================== */
void Motor_Init(void);                  /* 初始化电机GPIO和位置状态 */
void Motor_Step(uint8_t phase);         /* 输出一个8拍相位 */
void Motor_Step_Forward(void);          /* 正向走一步，关闭度增加 */
void Motor_Step_Reverse(void);          /* 反向走一步，关闭度减小 */
void Motor_Stop(void);                  /* 停止电机并释放线圈 */
void Motor_MoveTo(uint8_t target_pos);  /* 移动到目标关闭度，范围0~100 */
void Motor_Open(void);                  /* 全开窗帘，对应0%关闭度 */
void Motor_Close(void);                 /* 全关窗帘，对应100%关闭度 */
void Motor_Task(void);                  /* 非阻塞电机任务，在SysTick中周期调用 */
uint8_t Motor_GetPosition(void);        /* 获取当前关闭度 */
uint8_t Motor_IsMoving(void);           /* 查询电机是否正在运行 */

#endif
