#ifndef	_SYSTICK_H_
#define	_SYSTICK_H_

#include "stm32f10x.h"
#include "OLED.h"
#include "OLED_Data.h"
#include "key.h"
#include "ldr.h"

/* ==================== 任务数量宏 ==================== */
#define TASK_NUM_MAX	(sizeof(TaskComps) / sizeof(TaskComps[0]))

/* ==================== 位带操作宏 ==================== */
#define BITBAND(addr, bitnum) ((addr & 0xF0000000)+0x2000000+((addr & 0xFFFFF)<<5)+(bitnum<<2))
#define MEM_ADDR(addr)  *((volatile unsigned long  *)(addr))
#define BIT_ADDR(addr, bitnum)   MEM_ADDR(BITBAND(addr, bitnum))

/* GPIO输出数据寄存器地址映射 */
#define GPIOA_ODR_Addr    (GPIOA_BASE+12)
#define GPIOB_ODR_Addr    (GPIOB_BASE+12)
#define GPIOC_ODR_Addr    (GPIOC_BASE+12)
#define GPIOD_ODR_Addr    (GPIOD_BASE+12)
#define GPIOE_ODR_Addr    (GPIOE_BASE+12)
#define GPIOF_ODR_Addr    (GPIOF_BASE+12)
#define GPIOG_ODR_Addr    (GPIOG_BASE+12)

/* GPIO输入数据寄存器地址映射 */
#define GPIOA_IDR_Addr    (GPIOA_BASE+8)
#define GPIOB_IDR_Addr    (GPIOB_BASE+8)
#define GPIOC_IDR_Addr    (GPIOC_BASE+8)
#define GPIOD_IDR_Addr    (GPIOD_BASE+8)
#define GPIOE_IDR_Addr    (GPIOE_BASE+8)
#define GPIOF_IDR_Addr    (GPIOF_BASE+8)
#define GPIOG_IDR_Addr    (GPIOG_BASE+8)

/* GPIO位带读写宏 */
#define PAout(n)   BIT_ADDR(GPIOA_ODR_Addr,n)  /* PA输出 */
#define PAin(n)    BIT_ADDR(GPIOA_IDR_Addr,n)  /* PA输入 */
#define PBout(n)   BIT_ADDR(GPIOB_ODR_Addr,n)  /* PB输出 */
#define PBin(n)    BIT_ADDR(GPIOB_IDR_Addr,n)  /* PB输入 */
#define PCout(n)   BIT_ADDR(GPIOC_ODR_Addr,n)  /* PC输出 */
#define PCin(n)    BIT_ADDR(GPIOC_IDR_Addr,n)  /* PC输入 */
#define PDout(n)   BIT_ADDR(GPIOD_ODR_Addr,n)  /* PD输出 */
#define PDin(n)    BIT_ADDR(GPIOD_IDR_Addr,n)  /* PD输入 */
#define PEout(n)   BIT_ADDR(GPIOE_ODR_Addr,n)  /* PE输出 */
#define PEin(n)    BIT_ADDR(GPIOE_IDR_Addr,n)  /* PE输入 */
#define PFout(n)   BIT_ADDR(GPIOF_ODR_Addr,n)  /* PF输出 */
#define PFin(n)    BIT_ADDR(GPIOF_IDR_Addr,n)  /* PF输入 */
#define PGout(n)   BIT_ADDR(GPIOG_ODR_Addr,n)  /* PG输出 */
#define PGin(n)    BIT_ADDR(GPIOG_IDR_Addr,n)  /* PG输入 */

/* ==================== 时间片任务结构 ==================== */
typedef struct
{
	uint8_t run;               /* 运行标志 */
	uint16_t timCount;         /* 当前倒计数 */
	uint16_t timRload;         /* 重装载值 */
	void (*pTaskFunc)(void);   /* 任务回调函数 */
} TaskComps_t;

/* ==================== 函数声明 ==================== */
void TaskScheduleCb(void);     /* 时间片计数处理 */
void TaskSchedulCbReg(void (*fun)(void));  /* 注册时间片回调 */
void Task_Handle(void);        /* 预留任务处理接口 */
uint32_t Get_SysRunTime(void); /* 预留运行时间接口 */
void SysTick_Handler(void);    /* 1ms SysTick中断服务函数 */

/* ==================== 全局变量声明 ==================== */
extern uint64_t SysCount;      /* 系统运行毫秒计数 */
extern uint8_t Charge_Flag;    /* 预留标志 */
extern uint8_t Charge_TIM_M;   /* 预留分钟计数 */
extern uint8_t Charge_TIM_H;   /* 预留小时计数 */

#endif
