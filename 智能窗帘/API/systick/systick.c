/**
  ******************************************************************************
  * @file    systick.c
  * @brief   系统1ms节拍、按键去抖和步进电机周期任务入口。
  * 作者：程磊
  * 时间：2026-04-27 16:35:27 +08:00
  ******************************************************************************
  */

#include "systick.h"

/* ==================== 简单时间片任务表（当前仅保留扩展接口） ==================== */
static TaskComps_t TaskComps[] =
{
	/* {运行标志, 当前计数, 重装值, 回调函数} */
	{0, 5000, 5000, 0},
};

/* ==================== 全局系统计时变量 ==================== */
uint8_t  Charge_Flag  = 0;   /* 预留标志，当前未使用 */
uint8_t  Charge_TIM_M = 0;   /* 预留分钟计数 */
uint8_t  Charge_TIM_H = 0;   /* 预留小时计数 */
uint16_t Charge_TIM_ms= 0;   /* 预留毫秒计数 */
uint64_t SysCount      = 0;  /* 系统运行毫秒计数 */
uint32_t temp_sys      = 0;  /* 预留临时计数 */

static void (*hc_pTaskScheduleFunc)(void);  /* 预留任务回调 */

/**
  * @brief  注册时间片任务回调，当前工程保留接口。
  */
void TaskSchedulCbReg(void (*fun)(void))
{
	hc_pTaskScheduleFunc = fun;
}

/**
  * @brief  时间片任务计数递减，当前主流程未使用。
  */
void TaskScheduleCb(void)
{
	for(int i=0;i<TASK_NUM_MAX;i++)
	{
		if(TaskComps[i].timCount)
		{
			TaskComps[i].timCount--;
			if(TaskComps[i].timCount==0)
			{
				TaskComps[i].run=1;
				TaskComps[i].timCount=TaskComps[i].timRload;
			}
		}
	}
}

/**
  * @brief  SysTick中断服务函数，每1ms进入一次。
  * @note   在中断内只做轻量任务：系统计时、按键去抖、电机步进调度。
  */
void SysTick_Handler(void)
{
	SysCount++;
	Key_Check();    /* 按键10ms去抖状态机 */
	Motor_Task();   /* 步进电机非阻塞运行任务 */
}
