#include "stm32f10x.h"
#include "OLED.h"
#include <string.h>
#include <math.h>
#include <stdio.h>
#include <stdarg.h>

/* OLED底层驱动说明 */


/* OLED底层驱动说明 */

/* OLED底层驱动说明 */
uint8_t OLED_DisplayBuf[8][128];

/* OLED底层驱动说明 */


/* OLED底层驱动说明 */

/* OLED底层驱动说明 */
void OLED_W_SCL(uint8_t BitValue)
{
    /* OLED底层驱动说明 */
    GPIO_WriteBit(OLED_GPIO_SCL_PORT, OLED_GPIO_SCL_PIN, (BitAction)BitValue);
    
    /* OLED底层驱动说明 */
    //...
}

/* OLED底层驱动说明 */
void OLED_W_SDA(uint8_t BitValue)
{
    /* OLED底层驱动说明 */
    GPIO_WriteBit(OLED_GPIO_SDA_PORT, OLED_GPIO_SDA_PIN, (BitAction)BitValue);
    
    /* OLED底层驱动说明 */
    //...
}

/* OLED底层驱动说明 */
void OLED_GPIO_Init(void)
{
    uint32_t i, j;

    /* OLED底层驱动说明 */
    for (i = 0; i < 1000; i ++)
    {
        for (j = 0; j < 1000; j ++);
    }

    /* OLED底层驱动说明 */
    RCC_APB2PeriphClockCmd(OLED_GPIO_SCL_CLK | OLED_GPIO_SDA_CLK, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin = OLED_GPIO_SCL_PIN;
    GPIO_Init(OLED_GPIO_SCL_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = OLED_GPIO_SDA_PIN;
    GPIO_Init(OLED_GPIO_SDA_PORT, &GPIO_InitStructure);

    /* OLED底层驱动说明 */
    OLED_W_SCL(1);
    OLED_W_SDA(1);

    OLED_Clear();
    OLED_Update();
}

/* OLED底层驱动说明 */


/* OLED底层驱动说明 */

/* OLED底层驱动说明 */
void OLED_I2C_Start(void)
{
    OLED_W_SDA(1);        // OLED底层驱动说明
    OLED_W_SCL(1);        // OLED底层驱动说明
    OLED_W_SDA(0);        // OLED底层驱动说明
    OLED_W_SCL(0);        // OLED底层驱动说明
}

/* OLED底层驱动说明 */
void OLED_I2C_Stop(void)
{
    OLED_W_SDA(0);        // OLED底层驱动说明
    OLED_W_SCL(1);        // OLED底层驱动说明
    OLED_W_SDA(1);        // OLED底层驱动说明
}

/* OLED底层驱动说明 */
void OLED_I2C_SendByte(uint8_t Byte)
{
    uint8_t i;
    
    /* OLED底层驱动说明 */
    for (i = 0; i < 8; i++)
    {
        /* OLED底层驱动说明 */
        /* OLED底层驱动说明 */
        OLED_W_SDA(!!(Byte & (0x80 >> i)));
        OLED_W_SCL(1);    // OLED底层驱动说明
        OLED_W_SCL(0);    // OLED底层驱动说明
    }
    
    OLED_W_SCL(1);        // OLED底层驱动说明
    OLED_W_SCL(0);
}

/* OLED底层驱动说明 */
void OLED_WriteCommand(uint8_t Command)
{
    OLED_I2C_Start();                // OLED底层驱动说明
    OLED_I2C_SendByte(0x78);        // OLED底层驱动说明
    OLED_I2C_SendByte(0x00);        // OLED底层驱动说明
    OLED_I2C_SendByte(Command);        // OLED底层驱动说明
    OLED_I2C_Stop();                // OLED底层驱动说明
}

/* OLED底层驱动说明 */
void OLED_WriteData(uint8_t *Data, uint8_t Count)
{
    uint8_t i;
    
    OLED_I2C_Start();                // OLED底层驱动说明
    OLED_I2C_SendByte(0x78);        // OLED底层驱动说明
    OLED_I2C_SendByte(0x40);        // OLED底层驱动说明
    /* OLED底层驱动说明 */
    for (i = 0; i < Count; i ++)
    {
        OLED_I2C_SendByte(Data[i]);    // OLED底层驱动说明
    }
    OLED_I2C_Stop();                // OLED底层驱动说明
}

/* OLED底层驱动说明 */


/* OLED底层驱动说明 */

/* OLED底层驱动说明 */
void OLED_Init(void)
{
    OLED_GPIO_Init();            // OLED底层驱动说明
    
    /* OLED底层驱动说明 */
    OLED_WriteCommand(0xAE);    // OLED底层驱动说明
    
    OLED_WriteCommand(0xD5);    // OLED底层驱动说明
    OLED_WriteCommand(0x80);    //0x00~0xFF
    
    OLED_WriteCommand(0xA8);    // OLED底层驱动说明
    OLED_WriteCommand(0x3F);    //0x0E~0x3F
    
    OLED_WriteCommand(0xD3);    // OLED底层驱动说明
    OLED_WriteCommand(0x00);    //0x00~0x7F
    
    OLED_WriteCommand(0x40);    // OLED底层驱动说明
    
    OLED_WriteCommand(0xA1);    // OLED底层驱动说明
    
    OLED_WriteCommand(0xC8);    // OLED底层驱动说明

    OLED_WriteCommand(0xDA);    // OLED底层驱动说明
    OLED_WriteCommand(0x12);
    
    OLED_WriteCommand(0x81);    // OLED底层驱动说明
    OLED_WriteCommand(0xCF);    //0x00~0xFF

    OLED_WriteCommand(0xD9);    // OLED底层驱动说明
    OLED_WriteCommand(0xF1);

    OLED_WriteCommand(0xDB);    // OLED底层驱动说明
    OLED_WriteCommand(0x30);

    OLED_WriteCommand(0xA4);    // OLED底层驱动说明

    OLED_WriteCommand(0xA6);    // OLED底层驱动说明

    OLED_WriteCommand(0x8D);    // OLED底层驱动说明
    OLED_WriteCommand(0x14);

    OLED_WriteCommand(0xAF);    // OLED底层驱动说明
    
    OLED_Clear();                // OLED底层驱动说明
    OLED_Update();                // OLED底层驱动说明
}

/* OLED底层驱动说明 */
void OLED_SetCursor(uint8_t Page, uint8_t X)
{
    /* OLED底层驱动说明 */
    /* OLED底层驱动说明 */
    /* OLED底层驱动说明 */
    /* OLED底层驱动说明 */
//    X += 2;
    
    /* OLED底层驱动说明 */
    OLED_WriteCommand(0xB0 | Page);                    // OLED底层驱动说明
    OLED_WriteCommand(0x10 | ((X & 0xF0) >> 4));    // OLED底层驱动说明
    OLED_WriteCommand(0x00 | (X & 0x0F));            // OLED底层驱动说明
}

/* OLED底层驱动说明 */


/* OLED底层驱动说明 */

/* OLED底层驱动说明 */

/* OLED底层驱动说明 */
uint32_t OLED_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;    // OLED底层驱动说明
    while (Y --)            // OLED底层驱动说明
    {
        Result *= X;        // OLED底层驱动说明
    }
    return Result;
}

/* OLED底层驱动说明 */
uint8_t OLED_pnpoly(uint8_t nvert, int16_t *vertx, int16_t *verty, int16_t testx, int16_t testy)
{
    int16_t i, j, c = 0;
    
    /* OLED底层驱动说明 */
    /* OLED底层驱动说明 */
    for (i = 0, j = nvert - 1; i < nvert; j = i++)
    {
        if (((verty[i] > testy) != (verty[j] > testy)) &&
            (testx < (vertx[j] - vertx[i]) * (testy - verty[i]) / (verty[j] - verty[i]) + vertx[i]))
        {
            c = !c;
        }
    }
    return c;
}

/* OLED底层驱动说明 */
uint8_t OLED_IsInAngle(int16_t X, int16_t Y, int16_t StartAngle, int16_t EndAngle)
{
    int16_t PointAngle;
    PointAngle = atan2(Y, X) / 3.14 * 180;    // OLED底层驱动说明
    if (StartAngle < EndAngle)    // OLED底层驱动说明
    {
        /* OLED底层驱动说明 */
        if (PointAngle >= StartAngle && PointAngle <= EndAngle)
        {
            return 1;
        }
    }
    else            // OLED底层驱动说明
    {
        /* OLED底层驱动说明 */
        if (PointAngle >= StartAngle || PointAngle <= EndAngle)
        {
            return 1;
        }
    }
    return 0;        // OLED底层驱动说明
}

/* OLED底层驱动说明 */


/* OLED底层驱动说明 */

/* OLED底层驱动说明 */
void OLED_Update(void)
{
    uint8_t j;
    /* OLED底层驱动说明 */
    for (j = 0; j < 8; j ++)
    {
        /* OLED底层驱动说明 */
        OLED_SetCursor(j, 0);
        /* OLED底层驱动说明 */
        OLED_WriteData(OLED_DisplayBuf[j], 128);
    }
}

/* OLED底层驱动说明 */
void OLED_UpdateArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
    int16_t j;
    int16_t Page, Page1;
    
    /* OLED底层驱动说明 */
    /* OLED底层驱动说明 */
    Page = Y / 8;
    Page1 = (Y + Height - 1) / 8 + 1;
    if (Y < 0)
    {
        Page -= 1;
        Page1 -= 1;
    }
    
    /* OLED底层驱动说明 */
    for (j = Page; j < Page1; j ++)
    {
        if (X >= 0 && X <= 127 && j >= 0 && j <= 7)        // OLED底层驱动说明
        {
            /* OLED底层驱动说明 */
            OLED_SetCursor(j, X);
            /* OLED底层驱动说明 */
            OLED_WriteData(&OLED_DisplayBuf[j][X], Width);
        }
    }
}

/* OLED底层驱动说明 */
void OLED_Clear(void)
{
    uint8_t i, j;
    for (j = 0; j < 8; j ++)                // OLED底层驱动说明
    {
        for (i = 0; i < 128; i ++)            // OLED底层驱动说明
        {
            OLED_DisplayBuf[j][i] = 0x00;    // OLED底层驱动说明
        }
    }
}

/* OLED底层驱动说明 */
void OLED_ClearArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
    int16_t i, j;
    
    for (j = Y; j < Y + Height; j ++)        // OLED底层驱动说明
    {
        for (i = X; i < X + Width; i ++)    // OLED底层驱动说明
        {
            if (i >= 0 && i <= 127 && j >=0 && j <= 63)                // OLED底层驱动说明
            {
                OLED_DisplayBuf[j / 8][i] &= ~(0x01 << (j % 8));    // OLED底层驱动说明
            }
        }
    }
}

/* OLED底层驱动说明 */
void OLED_Reverse(void)
{
    uint8_t i, j;
    for (j = 0; j < 8; j ++)                // OLED底层驱动说明
    {
        for (i = 0; i < 128; i ++)            // OLED底层驱动说明
        {
            OLED_DisplayBuf[j][i] ^= 0xFF;    // OLED底层驱动说明
        }
    }
}
    
/* OLED底层驱动说明 */
void OLED_ReverseArea(int16_t X, int16_t Y, uint8_t Width, uint8_t Height)
{
    int16_t i, j;
    
    for (j = Y; j < Y + Height; j ++)        // OLED底层驱动说明
    {
        for (i = X; i < X + Width; i ++)    // OLED底层驱动说明
        {
            if (i >= 0 && i <= 127 && j >=0 && j <= 63)            // OLED底层驱动说明
            {
                OLED_DisplayBuf[j / 8][i] ^= 0x01 << (j % 8);    // OLED底层驱动说明
            }
        }
    }
}

/* OLED底层驱动说明 */
void OLED_ShowChar(int16_t X, int16_t Y, char Char, uint8_t FontSize)
{
    if (FontSize == OLED_8X16)        // OLED底层驱动说明
    {
        /* OLED底层驱动说明 */
        OLED_ShowImage(X, Y, 8, 16, OLED_F8x16[Char - ' ']);
    }
    else if(FontSize == OLED_6X8)    // OLED底层驱动说明
    {
        /* OLED底层驱动说明 */
        OLED_ShowImage(X, Y, 6, 8, OLED_F6x8[Char - ' ']);
    }
}

/* OLED底层驱动说明 */
void OLED_ShowString(int16_t X, int16_t Y, char *String, uint8_t FontSize)
{
    uint16_t i = 0;
    char SingleChar[5];
    uint8_t CharLength = 0;
    uint16_t XOffset = 0;
    uint16_t pIndex;
    
    while (String[i] != '\0')    // OLED底层驱动说明
    {
        
#ifdef OLED_CHARSET_UTF8                        // OLED底层驱动说明
        /* OLED底层驱动说明 */
        /* OLED底层驱动说明 */
        if ((String[i] & 0x80) == 0x00)            // OLED底层驱动说明
        {
            CharLength = 1;                        // OLED底层驱动说明
            SingleChar[0] = String[i ++];        // OLED底层驱动说明
            SingleChar[1] = '\0';                // OLED底层驱动说明
        }
        else if ((String[i] & 0xE0) == 0xC0)    // OLED底层驱动说明
        {
            CharLength = 2;                        // OLED底层驱动说明
            SingleChar[0] = String[i ++];        // OLED底层驱动说明
            if (String[i] == '\0') {break;}        // OLED底层驱动说明
            SingleChar[1] = String[i ++];        // OLED底层驱动说明
            SingleChar[2] = '\0';                // OLED底层驱动说明
        }
        else if ((String[i] & 0xF0) == 0xE0)    // OLED底层驱动说明
        {
            CharLength = 3;                        // OLED底层驱动说明
            SingleChar[0] = String[i ++];
            if (String[i] == '\0') {break;}
            SingleChar[1] = String[i ++];
            if (String[i] == '\0') {break;}
            SingleChar[2] = String[i ++];
            SingleChar[3] = '\0';
        }
        else if ((String[i] & 0xF8) == 0xF0)    // OLED底层驱动说明
        {
            CharLength = 4;                        // OLED底层驱动说明
            SingleChar[0] = String[i ++];
            if (String[i] == '\0') {break;}
            SingleChar[1] = String[i ++];
            if (String[i] == '\0') {break;}
            SingleChar[2] = String[i ++];
            if (String[i] == '\0') {break;}
            SingleChar[3] = String[i ++];
            SingleChar[4] = '\0';
        }
        else
        {
            i ++;            // OLED底层驱动说明
            continue;
        }
#endif
        
#ifdef OLED_CHARSET_GB2312                        // OLED底层驱动说明
        /* OLED底层驱动说明 */
        /* OLED底层驱动说明 */
        if ((String[i] & 0x80) == 0x00)            // OLED底层驱动说明
        {
            CharLength = 1;                        // OLED底层驱动说明
            SingleChar[0] = String[i ++];        // OLED底层驱动说明
            SingleChar[1] = '\0';                // OLED底层驱动说明
        }
        else                                    // OLED底层驱动说明
        {
            CharLength = 2;                        // OLED底层驱动说明
            SingleChar[0] = String[i ++];        // OLED底层驱动说明
            if (String[i] == '\0') {break;}        // OLED底层驱动说明
            SingleChar[1] = String[i ++];        // OLED底层驱动说明
            SingleChar[2] = '\0';                // OLED底层驱动说明
        }
#endif
        
        /* OLED底层驱动说明 */
        if (CharLength == 1)    // OLED底层驱动说明
        {
            /* OLED底层驱动说明 */
            OLED_ShowChar(X + XOffset, Y, SingleChar[0], FontSize);
            XOffset += FontSize;
        }
        else                    // OLED底层驱动说明
        {
            /* OLED底层驱动说明 */
            /* OLED底层驱动说明 */
            for (pIndex = 0; strcmp(OLED_CF16x16[pIndex].Index, "") != 0; pIndex ++)
            {
                /* OLED底层驱动说明 */
                if (strcmp(OLED_CF16x16[pIndex].Index, SingleChar) == 0)
                {
                    break;        // OLED底层驱动说明
                }
            }
            if (FontSize == OLED_8X16)        // OLED底层驱动说明
            {
                /* OLED底层驱动说明 */
                OLED_ShowImage(X + XOffset, Y, 16, 16, OLED_CF16x16[pIndex].Data);
                XOffset += 16;
            }
            else if (FontSize == OLED_6X8)    // OLED底层驱动说明
            {
                /* OLED底层驱动说明 */
                OLED_ShowChar(X + XOffset, Y, '?', OLED_6X8);
                XOffset += OLED_6X8;
            }
        }
    }
}

/* OLED底层驱动说明 */
void OLED_ShowNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i;
    for (i = 0; i < Length; i++)        // OLED底层驱动说明
    {
        /* OLED底层驱动说明 */
        /* OLED底层驱动说明 */
        /* OLED底层驱动说明 */
        OLED_ShowChar(X + i * FontSize, Y, Number / OLED_Pow(10, Length - i - 1) % 10 + '0', FontSize);
    }
}

/* OLED底层驱动说明 */
void OLED_ShowSignedNum(int16_t X, int16_t Y, int32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i;
    uint32_t Number1;
    
    if (Number >= 0)                        // OLED底层驱动说明
    {
        OLED_ShowChar(X, Y, '+', FontSize);    // OLED底层驱动说明
        Number1 = Number;                    // OLED底层驱动说明
    }
    else                                    // OLED底层驱动说明
    {
        OLED_ShowChar(X, Y, '-', FontSize);    // OLED底层驱动说明
        Number1 = -Number;                    // OLED底层驱动说明
    }
    
    for (i = 0; i < Length; i++)            // OLED底层驱动说明
    {
        /* OLED底层驱动说明 */
        /* OLED底层驱动说明 */
        /* OLED底层驱动说明 */
        OLED_ShowChar(X + (i + 1) * FontSize, Y, Number1 / OLED_Pow(10, Length - i - 1) % 10 + '0', FontSize);
    }
}

/* OLED底层驱动说明 */
void OLED_ShowHexNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i, SingleNumber;
    for (i = 0; i < Length; i++)        // OLED底层驱动说明
    {
        /* OLED底层驱动说明 */
        SingleNumber = Number / OLED_Pow(16, Length - i - 1) % 16;
        
        if (SingleNumber < 10)            // OLED底层驱动说明
        {
            /* OLED底层驱动说明 */
            /* OLED底层驱动说明 */
            OLED_ShowChar(X + i * FontSize, Y, SingleNumber + '0', FontSize);
        }
        else                            // OLED底层驱动说明
        {
            /* OLED底层驱动说明 */
            /* OLED底层驱动说明 */
            OLED_ShowChar(X + i * FontSize, Y, SingleNumber - 10 + 'A', FontSize);
        }
    }
}

/* OLED底层驱动说明 */
void OLED_ShowBinNum(int16_t X, int16_t Y, uint32_t Number, uint8_t Length, uint8_t FontSize)
{
    uint8_t i;
    for (i = 0; i < Length; i++)        // OLED底层驱动说明
    {
        /* OLED底层驱动说明 */
        /* OLED底层驱动说明 */
        /* OLED底层驱动说明 */
        OLED_ShowChar(X + i * FontSize, Y, Number / OLED_Pow(2, Length - i - 1) % 2 + '0', FontSize);
    }
}

/* OLED底层驱动说明 */
void OLED_ShowFloatNum(int16_t X, int16_t Y, double Number, uint8_t IntLength, uint8_t FraLength, uint8_t FontSize)
{
    uint32_t PowNum, IntNum, FraNum;
    
    if (Number >= 0)                        // OLED底层驱动说明
    {
        OLED_ShowChar(X, Y, '+', FontSize);    // OLED底层驱动说明
    }
    else                                    // OLED底层驱动说明
    {
        OLED_ShowChar(X, Y, '-', FontSize);    // OLED底层驱动说明
        Number = -Number;                    // OLED底层驱动说明
    }
    
    /* OLED底层驱动说明 */
    IntNum = Number;                        // OLED底层驱动说明
    Number -= IntNum;                        // OLED底层驱动说明
    PowNum = OLED_Pow(10, FraLength);        // OLED底层驱动说明
    FraNum = round(Number * PowNum);        // OLED底层驱动说明
    IntNum += FraNum / PowNum;                // OLED底层驱动说明
    
    /* OLED底层驱动说明 */
    OLED_ShowNum(X + FontSize, Y, IntNum, IntLength, FontSize);
    
    /* OLED底层驱动说明 */
    OLED_ShowChar(X + (IntLength + 1) * FontSize, Y, '.', FontSize);
    
    /* OLED底层驱动说明 */
    OLED_ShowNum(X + (IntLength + 2) * FontSize, Y, FraNum, FraLength, FontSize);
}

/* OLED底层驱动说明 */
void OLED_ShowImage(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, const uint8_t *Image)
{
    uint8_t i = 0, j = 0;
    int16_t Page, Shift;
    
    /* OLED底层驱动说明 */
    OLED_ClearArea(X, Y, Width, Height);
    
    /* OLED底层驱动说明 */
    /* OLED底层驱动说明 */
    for (j = 0; j < (Height - 1) / 8 + 1; j ++)
    {
        /* OLED底层驱动说明 */
        for (i = 0; i < Width; i ++)
        {
            if (X + i >= 0 && X + i <= 127)        // OLED底层驱动说明
            {
                /* OLED底层驱动说明 */
                Page = Y / 8;
                Shift = Y % 8;
                if (Y < 0)
                {
                    Page -= 1;
                    Shift += 8;
                }
                
                if (Page + j >= 0 && Page + j <= 7)        // OLED底层驱动说明
                {
                    /* OLED底层驱动说明 */
                    OLED_DisplayBuf[Page + j][X + i] |= Image[j * Width + i] << (Shift);
                }
                
                if (Page + j + 1 >= 0 && Page + j + 1 <= 7)        // OLED底层驱动说明
                {                    
                    /* OLED底层驱动说明 */
                    OLED_DisplayBuf[Page + j + 1][X + i] |= Image[j * Width + i] >> (8 - Shift);
                }
            }
        }
    }
}

/* OLED底层驱动说明 */
void OLED_Printf(int16_t X, int16_t Y, uint8_t FontSize, char *format, ...)
{
    char String[256];                        // OLED底层驱动说明
    va_list arg;                            // OLED底层驱动说明
    va_start(arg, format);                    // OLED底层驱动说明
    vsprintf(String, format, arg);            // OLED底层驱动说明
    va_end(arg);                            // OLED底层驱动说明
    OLED_ShowString(X, Y, String, FontSize);// OLED底层驱动说明
}

/* OLED底层驱动说明 */
void OLED_DrawPoint(int16_t X, int16_t Y)
{
    if (X >= 0 && X <= 127 && Y >=0 && Y <= 63)        // OLED底层驱动说明
    {
        /* OLED底层驱动说明 */
        OLED_DisplayBuf[Y / 8][X] |= 0x01 << (Y % 8);
    }
}

/* OLED底层驱动说明 */
uint8_t OLED_GetPoint(int16_t X, int16_t Y)
{
    if (X >= 0 && X <= 127 && Y >=0 && Y <= 63)        // OLED底层驱动说明
    {
        /* OLED底层驱动说明 */
        if (OLED_DisplayBuf[Y / 8][X] & 0x01 << (Y % 8))
        {
            return 1;    // OLED底层驱动说明
        }
    }
    
    return 0;        // OLED底层驱动说明
}

/* OLED底层驱动说明 */
void OLED_DrawLine(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1)
{
    int16_t x, y, dx, dy, d, incrE, incrNE, temp;
    int16_t x0 = X0, y0 = Y0, x1 = X1, y1 = Y1;
    uint8_t yflag = 0, xyflag = 0;
    
    if (y0 == y1)        // OLED底层驱动说明
    {
        /* OLED底层驱动说明 */
        if (x0 > x1) {temp = x0; x0 = x1; x1 = temp;}
        
        /* OLED底层驱动说明 */
        for (x = x0; x <= x1; x ++)
        {
            OLED_DrawPoint(x, y0);    // OLED底层驱动说明
        }
    }
    else if (x0 == x1)    // OLED底层驱动说明
    {
        /* OLED底层驱动说明 */
        if (y0 > y1) {temp = y0; y0 = y1; y1 = temp;}
        
        /* OLED底层驱动说明 */
        for (y = y0; y <= y1; y ++)
        {
            OLED_DrawPoint(x0, y);    // OLED底层驱动说明
        }
    }
    else                // OLED底层驱动说明
    {
        /* OLED底层驱动说明 */
        /* OLED底层驱动说明 */
        /* OLED底层驱动说明 */
        
        if (x0 > x1)    // OLED底层驱动说明
        {
            /* OLED底层驱动说明 */
            /* OLED底层驱动说明 */
            temp = x0; x0 = x1; x1 = temp;
            temp = y0; y0 = y1; y1 = temp;
        }
        
        if (y0 > y1)    // OLED底层驱动说明
        {
            /* OLED底层驱动说明 */
            /* OLED底层驱动说明 */
            y0 = -y0;
            y1 = -y1;
            
            /* OLED底层驱动说明 */
            yflag = 1;
        }
        
        if (y1 - y0 > x1 - x0)    // OLED底层驱动说明
        {
            /* OLED底层驱动说明 */
            /* OLED底层驱动说明 */
            temp = x0; x0 = y0; y0 = temp;
            temp = x1; x1 = y1; y1 = temp;
            
            /* OLED底层驱动说明 */
            xyflag = 1;
        }
        
        /* OLED底层驱动说明 */
        /* OLED底层驱动说明 */
        dx = x1 - x0;
        dy = y1 - y0;
        incrE = 2 * dy;
        incrNE = 2 * (dy - dx);
        d = 2 * dy - dx;
        x = x0;
        y = y0;
        
        /* OLED底层驱动说明 */
        if (yflag && xyflag){OLED_DrawPoint(y, -x);}
        else if (yflag)        {OLED_DrawPoint(x, -y);}
        else if (xyflag)    {OLED_DrawPoint(y, x);}
        else                {OLED_DrawPoint(x, y);}
        
        while (x < x1)        // OLED底层驱动说明
        {
            x ++;
            if (d < 0)        // OLED底层驱动说明
            {
                d += incrE;
            }
            else            // OLED底层驱动说明
            {
                y ++;
                d += incrNE;
            }
            
            /* OLED底层驱动说明 */
            if (yflag && xyflag){OLED_DrawPoint(y, -x);}
            else if (yflag)        {OLED_DrawPoint(x, -y);}
            else if (xyflag)    {OLED_DrawPoint(y, x);}
            else                {OLED_DrawPoint(x, y);}
        }    
    }
}

/* OLED底层驱动说明 */
void OLED_DrawRectangle(int16_t X, int16_t Y, uint8_t Width, uint8_t Height, uint8_t IsFilled)
{
    int16_t i, j;
    if (!IsFilled)        // OLED底层驱动说明
    {
        /* OLED底层驱动说明 */
        for (i = X; i < X + Width; i ++)
        {
            OLED_DrawPoint(i, Y);
            OLED_DrawPoint(i, Y + Height - 1);
        }
        /* OLED底层驱动说明 */
        for (i = Y; i < Y + Height; i ++)
        {
            OLED_DrawPoint(X, i);
            OLED_DrawPoint(X + Width - 1, i);
        }
    }
    else                // OLED底层驱动说明
    {
        /* OLED底层驱动说明 */
        for (i = X; i < X + Width; i ++)
        {
            /* OLED底层驱动说明 */
            for (j = Y; j < Y + Height; j ++)
            {
                /* OLED底层驱动说明 */
                OLED_DrawPoint(i, j);
            }
        }
    }
}

/* OLED底层驱动说明 */
void OLED_DrawTriangle(int16_t X0, int16_t Y0, int16_t X1, int16_t Y1, int16_t X2, int16_t Y2, uint8_t IsFilled)
{
    int16_t minx = X0, miny = Y0, maxx = X0, maxy = Y0;
    int16_t i, j;
    int16_t vx[] = {X0, X1, X2};
    int16_t vy[] = {Y0, Y1, Y2};
    
    if (!IsFilled)            // OLED底层驱动说明
    {
        /* OLED底层驱动说明 */
        OLED_DrawLine(X0, Y0, X1, Y1);
        OLED_DrawLine(X0, Y0, X2, Y2);
        OLED_DrawLine(X1, Y1, X2, Y2);
    }
    else                    // OLED底层驱动说明
    {
        /* OLED底层驱动说明 */
        if (X1 < minx) {minx = X1;}
        if (X2 < minx) {minx = X2;}
        if (Y1 < miny) {miny = Y1;}
        if (Y2 < miny) {miny = Y2;}
        
        /* OLED底层驱动说明 */
        if (X1 > maxx) {maxx = X1;}
        if (X2 > maxx) {maxx = X2;}
        if (Y1 > maxy) {maxy = Y1;}
        if (Y2 > maxy) {maxy = Y2;}
        
        /* OLED底层驱动说明 */
        /* OLED底层驱动说明 */
        /* OLED底层驱动说明 */        
        for (i = minx; i <= maxx; i ++)
        {
            /* OLED底层驱动说明 */    
            for (j = miny; j <= maxy; j ++)
            {
                /* OLED底层驱动说明 */
                /* OLED底层驱动说明 */
                if (OLED_pnpoly(3, vx, vy, i, j)) {OLED_DrawPoint(i, j);}
            }
        }
    }
}

/* OLED底层驱动说明 */
void OLED_DrawCircle(int16_t X, int16_t Y, uint8_t Radius, uint8_t IsFilled)
{
    int16_t x, y, d, j;
    
    /* OLED底层驱动说明 */
    /* OLED底层驱动说明 */
    /* OLED底层驱动说明 */
    
    d = 1 - Radius;
    x = 0;
    y = Radius;
    
    /* OLED底层驱动说明 */
    OLED_DrawPoint(X + x, Y + y);
    OLED_DrawPoint(X - x, Y - y);
    OLED_DrawPoint(X + y, Y + x);
    OLED_DrawPoint(X - y, Y - x);
    
    if (IsFilled)        // OLED底层驱动说明
    {
        /* OLED底层驱动说明 */
        for (j = -y; j < y; j ++)
        {
            /* OLED底层驱动说明 */
            OLED_DrawPoint(X, Y + j);
        }
    }
    
    while (x < y)        // OLED底层驱动说明
    {
        x ++;
        if (d < 0)        // OLED底层驱动说明
        {
            d += 2 * x + 1;
        }
        else            // OLED底层驱动说明
        {
            y --;
            d += 2 * (x - y) + 1;
        }
        
        /* OLED底层驱动说明 */
        OLED_DrawPoint(X + x, Y + y);
        OLED_DrawPoint(X + y, Y + x);
        OLED_DrawPoint(X - x, Y - y);
        OLED_DrawPoint(X - y, Y - x);
        OLED_DrawPoint(X + x, Y - y);
        OLED_DrawPoint(X + y, Y - x);
        OLED_DrawPoint(X - x, Y + y);
        OLED_DrawPoint(X - y, Y + x);
        
        if (IsFilled)    // OLED底层驱动说明
        {
            /* OLED底层驱动说明 */
            for (j = -y; j < y; j ++)
            {
                /* OLED底层驱动说明 */
                OLED_DrawPoint(X + x, Y + j);
                OLED_DrawPoint(X - x, Y + j);
            }
            
            /* OLED底层驱动说明 */
            for (j = -x; j < x; j ++)
            {
                /* OLED底层驱动说明 */
                OLED_DrawPoint(X - y, Y + j);
                OLED_DrawPoint(X + y, Y + j);
            }
        }
    }
}

/* OLED底层驱动说明 */
void OLED_DrawEllipse(int16_t X, int16_t Y, uint8_t A, uint8_t B, uint8_t IsFilled)
{
    int16_t x, y, j;
    int16_t a = A, b = B;
    float d1, d2;
    
    /* OLED底层驱动说明 */
    /* OLED底层驱动说明 */
    
    x = 0;
    y = b;
    d1 = b * b + a * a * (-b + 0.5);
    
    if (IsFilled)    // OLED底层驱动说明
    {
        /* OLED底层驱动说明 */
        for (j = -y; j < y; j ++)
        {
            /* OLED底层驱动说明 */
            OLED_DrawPoint(X, Y + j);
            OLED_DrawPoint(X, Y + j);
        }
    }
    
    /* OLED底层驱动说明 */
    OLED_DrawPoint(X + x, Y + y);
    OLED_DrawPoint(X - x, Y - y);
    OLED_DrawPoint(X - x, Y + y);
    OLED_DrawPoint(X + x, Y - y);
    
    /* OLED底层驱动说明 */
    while (b * b * (x + 1) < a * a * (y - 0.5))
    {
        if (d1 <= 0)        // OLED底层驱动说明
        {
            d1 += b * b * (2 * x + 3);
        }
        else                // OLED底层驱动说明
        {
            d1 += b * b * (2 * x + 3) + a * a * (-2 * y + 2);
            y --;
        }
        x ++;
        
        if (IsFilled)    // OLED底层驱动说明
        {
            /* OLED底层驱动说明 */
            for (j = -y; j < y; j ++)
            {
                /* OLED底层驱动说明 */
                OLED_DrawPoint(X + x, Y + j);
                OLED_DrawPoint(X - x, Y + j);
            }
        }
        
        /* OLED底层驱动说明 */
        OLED_DrawPoint(X + x, Y + y);
        OLED_DrawPoint(X - x, Y - y);
        OLED_DrawPoint(X - x, Y + y);
        OLED_DrawPoint(X + x, Y - y);
    }
    
    /* OLED底层驱动说明 */
    d2 = b * b * (x + 0.5) * (x + 0.5) + a * a * (y - 1) * (y - 1) - a * a * b * b;
    
    while (y > 0)
    {
        if (d2 <= 0)        // OLED底层驱动说明
        {
            d2 += b * b * (2 * x + 2) + a * a * (-2 * y + 3);
            x ++;
            
        }
        else                // OLED底层驱动说明
        {
            d2 += a * a * (-2 * y + 3);
        }
        y --;
        
        if (IsFilled)    // OLED底层驱动说明
        {
            /* OLED底层驱动说明 */
            for (j = -y; j < y; j ++)
            {
                /* OLED底层驱动说明 */
                OLED_DrawPoint(X + x, Y + j);
                OLED_DrawPoint(X - x, Y + j);
            }
        }
        
        /* OLED底层驱动说明 */
        OLED_DrawPoint(X + x, Y + y);
        OLED_DrawPoint(X - x, Y - y);
        OLED_DrawPoint(X - x, Y + y);
        OLED_DrawPoint(X + x, Y - y);
    }
}

/* OLED底层驱动说明 */
void OLED_DrawArc(int16_t X, int16_t Y, uint8_t Radius, int16_t StartAngle, int16_t EndAngle, uint8_t IsFilled)
{
    int16_t x, y, d, j;
    
    /* OLED底层驱动说明 */
    
    d = 1 - Radius;
    x = 0;
    y = Radius;
    
    /* OLED底层驱动说明 */
    if (OLED_IsInAngle(x, y, StartAngle, EndAngle))    {OLED_DrawPoint(X + x, Y + y);}
    if (OLED_IsInAngle(-x, -y, StartAngle, EndAngle)) {OLED_DrawPoint(X - x, Y - y);}
    if (OLED_IsInAngle(y, x, StartAngle, EndAngle)) {OLED_DrawPoint(X + y, Y + x);}
    if (OLED_IsInAngle(-y, -x, StartAngle, EndAngle)) {OLED_DrawPoint(X - y, Y - x);}
    
    if (IsFilled)    // OLED底层驱动说明
    {
        /* OLED底层驱动说明 */
        for (j = -y; j < y; j ++)
        {
            /* OLED底层驱动说明 */
            if (OLED_IsInAngle(0, j, StartAngle, EndAngle)) {OLED_DrawPoint(X, Y + j);}
        }
    }
    
    while (x < y)        // OLED底层驱动说明
    {
        x ++;
        if (d < 0)        // OLED底层驱动说明
        {
            d += 2 * x + 1;
        }
        else            // OLED底层驱动说明
        {
            y --;
            d += 2 * (x - y) + 1;
        }
        
        /* OLED底层驱动说明 */
        if (OLED_IsInAngle(x, y, StartAngle, EndAngle)) {OLED_DrawPoint(X + x, Y + y);}
        if (OLED_IsInAngle(y, x, StartAngle, EndAngle)) {OLED_DrawPoint(X + y, Y + x);}
        if (OLED_IsInAngle(-x, -y, StartAngle, EndAngle)) {OLED_DrawPoint(X - x, Y - y);}
        if (OLED_IsInAngle(-y, -x, StartAngle, EndAngle)) {OLED_DrawPoint(X - y, Y - x);}
        if (OLED_IsInAngle(x, -y, StartAngle, EndAngle)) {OLED_DrawPoint(X + x, Y - y);}
        if (OLED_IsInAngle(y, -x, StartAngle, EndAngle)) {OLED_DrawPoint(X + y, Y - x);}
        if (OLED_IsInAngle(-x, y, StartAngle, EndAngle)) {OLED_DrawPoint(X - x, Y + y);}
        if (OLED_IsInAngle(-y, x, StartAngle, EndAngle)) {OLED_DrawPoint(X - y, Y + x);}
        
        if (IsFilled)    // OLED底层驱动说明
        {
            /* OLED底层驱动说明 */
            for (j = -y; j < y; j ++)
            {
                /* OLED底层驱动说明 */
                if (OLED_IsInAngle(x, j, StartAngle, EndAngle)) {OLED_DrawPoint(X + x, Y + j);}
                if (OLED_IsInAngle(-x, j, StartAngle, EndAngle)) {OLED_DrawPoint(X - x, Y + j);}
            }
            
            /* OLED底层驱动说明 */
            for (j = -x; j < x; j ++)
            {
                /* OLED底层驱动说明 */
                if (OLED_IsInAngle(-y, j, StartAngle, EndAngle)) {OLED_DrawPoint(X - y, Y + j);}
                if (OLED_IsInAngle(y, j, StartAngle, EndAngle)) {OLED_DrawPoint(X + y, Y + j);}
            }
        }
    }
}

/* OLED底层驱动说明 */


/* OLED底层驱动说明 */
/*****************jiangxiekeji.com*****************/
