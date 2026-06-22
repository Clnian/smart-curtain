/**
  ******************************************************************************
  * @file    key.c
  * @brief   按键扫描、OLED页面显示、自动窗帘控制逻辑。
  *
  * 按键分配：
  *   KEY1 = PC13   第3页切换选中参数：温度阈值 / 光照阈值
  *   KEY2 = PC14   第3页减小参数；手动模式下控制关窗/停止
  *   KEY3 = PC15   第3页增大参数；手动模式下控制开窗/停止
  *   KEY4 = PA0    短按切换OLED页面，长按切换自动/手动模式
  *
  * 作者：程磊
  * 时间：2026-04-27 16:35:27 +08:00
  ******************************************************************************
  */

#include "key.h"
#include "motor.h"

/* SysCount由SysTick_Handler每1ms累加一次。 */
extern uint64_t SysCount;

/* ==================== 按键硬件状态变量 ==================== */
struct
{
    uint8_t key_state;    /* 当前GPIO电平状态 */
    uint8_t key_judge;    /* 去抖状态机：0=等待按下，1=确认按下，2=等待释放 */
    uint8_t key_flag;     /* 按键事件标志：1=有事件，0=无事件 */
    uint8_t key_reserve;  /* 预留 */
} key[4];

/* ==================== 全局状态变量 ==================== */
uint8_t  Key4_Turn_Down = 1;    /* 当前OLED页：1=首页，2=状态页，3=设置页 */
uint8_t  Select_Index   = 0;    /* 设置页光标：0=温度阈值，1=光照阈值 */

float    Tem_Up   = 30.0f;      /* 温度阈值，单位摄氏度 */
uint16_t ldr_Down = 500;        /* 光照阈值，单位Lux */
uint16_t Ldr_Hyst = 100;        /* 光照迟滞死区，单位Lux */

uint8_t  Auto_Mode   = 1;       /* 1=自动模式，0=手动模式 */

uint8_t  Cool_flag   = 0;       /* 预留标志，当前未使用 */
uint8_t  Power_flag  = 0;       /* 预留标志，当前未使用 */
uint8_t  Blower_flag = 0;       /* 预留标志，当前未使用 */

/* KEY4长短按追踪变量，依赖SysCount做时间判断。 */
static uint8_t  Key4_Pressed     = 0;  /* 0=未按下，1=正在按住 */
static uint8_t  Key4_Hold_Count  = 0;  /* 0=未触发长按，10=长按已触发 */
static uint64_t Key4_Press_Start = 0;  /* 按下时刻的SysCount值 */

/* ==================== Key_Init ==================== */
/**
  * @brief  按键GPIO初始化。
  * @note   KEY1~KEY3: PC13/PC14/PC15 上拉输入；
  *         KEY4: PA0 上拉输入；按下为低电平，释放为高电平。
  * 作者：程磊
  * 时间：2026-04-27 16:35:27 +08:00
  */
void Key_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOC, ENABLE);

    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

    /* KEY4: PA0 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* KEY1/KEY2/KEY3: PC13/PC14/PC15 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    /* 初始化所有按键状态。 */
    for (uint8_t i = 0; i < 4; i++)
    {
        key[i].key_state   = 1;
        key[i].key_judge   = 0;
        key[i].key_flag    = 0;
        key[i].key_reserve = 0;
    }
}

/* ==================== OLED_Select ==================== */
/**
  * @brief  切换并刷新OLED页面。
  * @param  select: 页面号，1=温湿度页，2=光照/窗帘状态页，3=阈值设置页。
  * @note   OLED使用GB2312中文字库显示中文，新增中文时需同步补充OLED_Data.c字模。
  * 作者：程磊
  * 时间：2026-04-27 16:35:27 +08:00
  */
void OLED_Select(uint8_t select)
{
    OLED_Clear();
    switch (select)
    {
        /* --------- 第1页：温度 / 湿度 / 云连接状态 --------- */
        case 1:
            OLED_Printf(0,  0, OLED_8X16, "智能窗帘控制系统");
            OLED_Printf(0, 16, OLED_8X16, "温度:%.1f℃", (float)Real_Tem);
            OLED_Printf(0, 32, OLED_8X16, "湿度:%.1f%%", (float)Real_Hum);
            OLED_Printf(0, 48, OLED_8X16, "%s",
                        (Esp8266_RecvData.connect_flag == WIFI_OK) ? "云服务已连接√" : "云服务未连接×");
            break;

        /* --------- 第2页：光照强度 + 窗帘位置/模式 --------- */
        case 2:
            OLED_Printf(0,  0, OLED_8X16, "智能窗帘控制系统");
            OLED_Printf(0, 16, OLED_8X16, "光照:%4d lx", ldr);
            /* 该行使用英文缩写，避免超过OLED 16字符宽度。 */
            OLED_Printf(0, 32, OLED_8X16, "Pos:%3d%% Md:%s",
                        Curtain_Position, Auto_Mode ? "Auto" : "Manu");
            break;

        /* --------- 第3页：阈值设置 --------- */
        case 3:
            OLED_Printf(0,  0, OLED_8X16, "智能窗帘控制系统");
            OLED_Printf(0, 16, OLED_8X16, "%s温度阈值:%.1f℃",
                        Select_Index == 0 ? ">" : " ", Tem_Up);
            OLED_Printf(0, 32, OLED_8X16, "%s光照阈值:%3d lx",
                        Select_Index == 1 ? ">" : " ", ldr_Down);
            /* 该行使用英文缩写，避免超过OLED 16字符宽度。 */
            OLED_Printf(0, 48, OLED_8X16, "Pos:%3d%% [%s]",
                        Curtain_Position, Auto_Mode ? "Auto" : "Manu");
            break;

        default:
            break;
    }
    OLED_Update();
}

/* ==================== Action_Key_Process ==================== */
/**
  * @brief  处理按键动作并刷新OLED，建议主循环每100ms调用一次。
  * @note   KEY4通过GPIO直读实现长短按，KEY1~KEY3通过Key_Check去抖。
  * 作者：程磊
  * 时间：2026-04-27 16:35:27 +08:00
  */
void Action_Key_Process(void)
{
    static uint8_t last_key3_pressed = 0;

    /* KEY4独立处理：短按切页，长按切换自动/手动模式。 */
    key[3].key_flag = 0;

    uint8_t pin4 = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);

    if (pin4 == 0)
    {
        if (!Key4_Pressed)
        {
            Key4_Pressed     = 1;
            Key4_Hold_Count  = 0;
            Key4_Press_Start = SysCount;
        }
        else if (Key4_Hold_Count == 0 &&
                 (SysCount - Key4_Press_Start >= 1000))
        {
            Key4_Hold_Count = 10;
            Auto_Mode       = !Auto_Mode;
            OLED_Select(Key4_Turn_Down);
        }
    }
    else
    {
        if (Key4_Pressed && Key4_Hold_Count < 10)
        {
            Key4_Turn_Down = (Key4_Turn_Down % 3) + 1;
            Select_Index   = 0;
            OLED_Select(Key4_Turn_Down);
        }
        Key4_Pressed    = 0;
        Key4_Hold_Count = 0;
    }

    uint8_t Key_value = Key_Scan();

    /* 第3页用于调整温度阈值和光照阈值。 */
    if (Key4_Turn_Down == 3)
    {
        if (Key_value == KEY1_PRESS)
        {
            Select_Index = (Select_Index + 1) % 2;
            OLED_Select(3);
        }

        if (Key_value == KEY2_PRESS)
        {
            if (Select_Index == 0)
            {
                Tem_Up -= 1.0f;
                if (Tem_Up < -20.0f) Tem_Up = -20.0f;
            }
            else
            {
                if (ldr_Down >= 50) ldr_Down -= 50;
                else ldr_Down = 0;
            }
            OLED_Select(3);
        }

        /* KEY3做边沿触发，避免同一次扫描重复加值。 */
        if (Key_value == KEY3_PRESS && !last_key3_pressed)
        {
            if (Select_Index == 0)
            {
                Tem_Up += 1.0f;
                if (Tem_Up > 60.0f) Tem_Up = 60.0f;
            }
            else
            {
                if (ldr_Down <= 950) ldr_Down += 50;
                else ldr_Down = 1000;
            }
            OLED_Select(3);
        }
    }

    last_key3_pressed = (Key_value == KEY3_PRESS);

    /* 手动模式下：KEY2关窗，KEY3开窗；电机运行时再次按下则停止。 */
    if (!Auto_Mode && Key4_Turn_Down != 3)
    {
        if (Key_value == KEY2_PRESS || Key_value == KEY3_PRESS)
        {
            if (Motor_IsMoving())
            {
                Motor_Stop();
            }
            else
            {
                if (Key_value == KEY2_PRESS) Motor_Close();
                else                         Motor_Open();
            }
        }
    }

    if      (Key4_Turn_Down == 1) OLED_Select(1);
    else if (Key4_Turn_Down == 2) OLED_Select(2);

    /* 第3页只在窗帘位置变化时刷新，避免设置页面频繁闪烁。 */
    if (Key4_Turn_Down == 3)
    {
        static uint8_t last_curtain_pos = 255;
        if (Curtain_Position != last_curtain_pos)
        {
            last_curtain_pos = Curtain_Position;
            OLED_Select(3);
        }
    }
}

/* ==================== Key_Check ==================== */
/**
  * @brief  按键去抖状态机，由SysTick_Handler每1ms调用，内部每10ms采样一次。
  * @note   状态0等待按下，状态1确认按下，状态2等待释放。
  * 作者：程磊
  * 时间：2026-04-27 16:35:27 +08:00
  */
void Key_Check(void)
{
    static uint8_t Tick_Count = 0;
    Tick_Count++;
    if (Tick_Count < 10) return;
    Tick_Count = 0;

    key[0].key_state = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_13);
    key[1].key_state = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_14);
    key[2].key_state = GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_15);
    key[3].key_state = GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0);

    for (uint8_t i = 0; i < 4; i++)
    {
        switch (key[i].key_judge)
        {
            case 0:
                if (key[i].key_state == 0) key[i].key_judge++;
                break;

            case 1:
                if (key[i].key_state == 0)
                {
                    key[i].key_judge++;
                    key[i].key_flag = 1;
                }
                else
                {
                    key[i].key_judge = 0;
                }
                break;

            case 2:
                if (key[i].key_state != 0) key[i].key_judge = 0;
                break;
        }
    }
}

/* ==================== Key_Scan ==================== */
/**
  * @brief  获取并清除一个按键事件。
  * @retval KEY1_PRESS、KEY2_PRESS、KEY3_PRESS、KEY4_PRESS，或0表示无事件。
  * 作者：程磊
  * 时间：2026-04-27 16:35:27 +08:00
  */
uint8_t Key_Scan(void)
{
    for (uint8_t i = 0; i < 4; i++)
    {
        if (key[i].key_flag)
        {
            key[i].key_flag = 0;
            switch (i)
            {
                case 0: return KEY1_PRESS;
                case 1: return KEY2_PRESS;
                case 2: return KEY3_PRESS;
                case 3: return KEY4_PRESS;
            }
        }
    }
    return 0;
}

/* ==================== alarm_int ==================== */
/**
  * @brief  温度/光照告警标志更新，并在自动模式下控制窗帘。
  * @note   自动策略：
  *         1. 夜间低光照 L < 150 lx 时强制关窗，保护隐私；
  *         2. 夏季模式 Real_Tem >= Tem_Up，强光关窗防晒，弱光开窗通风；
  *         3. 冬季模式 Real_Tem < Tem_Up，强光开窗采暖，弱光关窗保温；
  *         4. 光照阈值使用 Ldr_Hyst 迟滞区，避免临界点反复动作。
  * 作者：程磊
  * 时间：2026-04-27 16:35:27 +08:00
  */
void alarm_int(void)
{
    if (Real_Tem > Tem_Up)
    {
        led_set();
        Esp8266_RecvData.Mode_Real_Tem = 1;
    }
    else
    {
        Esp8266_RecvData.Mode_Real_Tem = 0;
    }

    if (ldr > ldr_Down)
    {
        led_set();
        Esp8266_RecvData.Mode_ldr = 1;
    }
    else
    {
        Esp8266_RecvData.Mode_ldr = 0;
    }

    if (!Auto_Mode) return;

    if (ldr < 150)
    {
        if (Curtain_Position < 95) Motor_Close();
        return;
    }

    int16_t L      = (int16_t)ldr;
    int16_t L_up   = (int16_t)ldr_Down + (int16_t)Ldr_Hyst;
    int16_t L_down = (int16_t)ldr_Down - (int16_t)Ldr_Hyst;
    if (L_down < 0) L_down = 0;

    if (Real_Tem >= Tem_Up)
    {
        if (L > L_up)
        {
            if (Curtain_Position < 95) Motor_Close();
        }
        else if (L < L_down)
        {
            if (Curtain_Position > 5) Motor_Open();
        }
    }
    else
    {
        if (L > L_up)
        {
            if (Curtain_Position > 5) Motor_Open();
        }
        else if (L < L_down)
        {
            if (Curtain_Position < 95) Motor_Close();
        }
    }
}
