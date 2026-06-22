/**
  ******************************************************************************
  * @file    motor.c
  * @brief   28BYJ-48步进电机窗帘控制驱动，使用ULN2003驱动板。
  * @note    位置语义统一为：Curtain_Position=0表示全开，100表示全关。
  * 作者：程磊
  * 时间：2026-04-27 16:35:27 +08:00
  ******************************************************************************
  */

#include "motor.h"

/* SysCount由SysTick_Handler每1ms累加，用于限制步进速度。 */
extern uint64_t SysCount;

/* 相邻两步之间的最小间隔。2ms约等于500步/秒，兼顾速度和可靠性。 */
#define MOTOR_STEP_MS     2

uint8_t Curtain_Position = 0;     /* 当前关闭度：0=全开，100=全关 */
#define MOTOR_FULL_STEPS  1000    /* 从全开到全关的标定步数 */

static uint8_t Motor_Direction = 0;      /* 0=停止，1=关闭方向，2=打开方向 */
static uint32_t Motor_Step_Count = 0;    /* 当前累计步数，0~MOTOR_FULL_STEPS */
static uint32_t Motor_Target_Steps = 0;  /* 目标步数 */
static uint8_t Motor_Is_Running = 0;     /* 电机运行状态 */

/* 28BYJ-48采用4相8拍励磁顺序：A-AB-B-BC-C-CD-D-DA。 */
const uint8_t MOTOR_PHASE_TABLE[8] = {
    0x01,  /* 0001 A相通电 */
    0x03,  /* 0011 AB相通电 */
    0x02,  /* 0010 B相通电 */
    0x06,  /* 0110 BC相通电 */
    0x04,  /* 0100 C相通电 */
    0x0C,  /* 1100 CD相通电 */
    0x08,  /* 1000 D相通电 */
    0x09   /* 1001 DA相通电 */
};

static uint8_t Motor_Phase_Index = 0;  /* 当前8拍相位索引 */

static void Motor_Single_Step_Internal(uint8_t dir);

/**
  * @brief  初始化电机控制GPIO和窗帘位置状态。
  */
void Motor_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = MOTOR_IN1_PIN | MOTOR_IN2_PIN |
                                    MOTOR_IN3_PIN | MOTOR_IN4_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    Motor_Stop();
    Curtain_Position = 0;
    Motor_Direction = 0;
    Motor_Step_Count = 0;
    Motor_Target_Steps = 0;
    Motor_Is_Running = 0;
}

/**
  * @brief  将8拍相位输出到ULN2003输入端。
  */
void Motor_Step(uint8_t phase)
{
    MOTOR_IN1(phase & 0x01);
    MOTOR_IN2(phase & 0x02);
    MOTOR_IN3(phase & 0x04);
    MOTOR_IN4(phase & 0x08);
}

/**
  * @brief  执行一次内部步进，并同步更新步数计数。
  * @param  dir 1=关闭方向，2=打开方向，0=停止。
  */
static void Motor_Single_Step_Internal(uint8_t dir)
{
    if (dir == 0) {
        Motor_Stop();
        return;
    }

    if (dir == 1) {
        Motor_Phase_Index = (Motor_Phase_Index + 1) % 8;
        Motor_Step(MOTOR_PHASE_TABLE[Motor_Phase_Index]);
        Motor_Step_Count++;
    } else {
        Motor_Phase_Index = (Motor_Phase_Index + 7) % 8;
        Motor_Step(MOTOR_PHASE_TABLE[Motor_Phase_Index]);
        if (Motor_Step_Count > 0) Motor_Step_Count--;
    }
}

void Motor_Step_Forward(void)
{
    Motor_Single_Step_Internal(1);
}

void Motor_Step_Reverse(void)
{
    Motor_Single_Step_Internal(2);
}

/**
  * @brief  停止电机并关闭四相输出，避免线圈持续发热。
  */
void Motor_Stop(void)
{
    MOTOR_IN1(0);
    MOTOR_IN2(0);
    MOTOR_IN3(0);
    MOTOR_IN4(0);
    Motor_Direction = 0;
    Motor_Is_Running = 0;
}

/**
  * @brief  设置目标关闭度，电机将在Motor_Task中非阻塞运行到目标位置。
  * @param  target_pos 目标关闭度，0=全开，100=全关。
  */
void Motor_MoveTo(uint8_t target_pos)
{
    if (target_pos > 100) target_pos = 100;
    uint32_t new_target = (uint32_t)target_pos * MOTOR_FULL_STEPS / 100;

    if (new_target > Motor_Step_Count) {
        Motor_Target_Steps = new_target;
        Motor_Direction    = 1;
        Motor_Is_Running   = 1;
    } else if (new_target < Motor_Step_Count) {
        Motor_Target_Steps = new_target;
        Motor_Direction    = 2;
        Motor_Is_Running   = 1;
    } else {
        Motor_Stop();
    }
}

void Motor_Open(void)
{
    Motor_MoveTo(0);
}

void Motor_Close(void)
{
    Motor_MoveTo(100);
}

/**
  * @brief  电机非阻塞运行任务。
  * @note   由SysTick_Handler每1ms调用，内部按MOTOR_STEP_MS限速。
  */
void Motor_Task(void)
{
    static uint64_t s_last_step_ms = 0;

    if (!Motor_Is_Running) return;

    if (SysCount - s_last_step_ms < MOTOR_STEP_MS) return;
    s_last_step_ms = SysCount;

    if (Motor_Direction == 1 && Motor_Step_Count < Motor_Target_Steps) {
        Motor_Single_Step_Internal(1);
    } else if (Motor_Direction == 2 && Motor_Step_Count > Motor_Target_Steps) {
        Motor_Single_Step_Internal(2);
    } else {
        Motor_Stop();
        Motor_Is_Running = 0;
    }

    Curtain_Position = (uint8_t)((Motor_Step_Count * 100) / MOTOR_FULL_STEPS);
    if (Curtain_Position > 100) Curtain_Position = 100;
}

uint8_t Motor_GetPosition(void)
{
    return Curtain_Position;
}

uint8_t Motor_IsMoving(void)
{
    return Motor_Is_Running;
}
