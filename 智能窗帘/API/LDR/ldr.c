/**
  ******************************************************************************
  * @file    ldr.c
  * @brief   光敏电阻采集驱动，通过ADC1通道4计算环境光照强度。
  * 作者：程磊
  * 时间：2026-04-27 16:35:27 +08:00
  ******************************************************************************
  */

#include "ldr.h"

/**
  * @brief  初始化光敏电阻ADC采集通道。
  * @note   PA4配置为模拟输入，ADC1单通道单次转换。
  */
void LDR_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);  /* ADC时钟72MHz/6=12MHz */

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_Cmd(ADC1, ENABLE);

    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1));
}

/**
  * @brief  读取指定ADC通道原始值。
  * @param  channel ADC通道号，光敏电阻使用通道4。
  * @retval 12位ADC采样值，范围0~4095。
  */
uint16_t LDR_ADC_Value(uint8_t channel)
{
    ADC_RegularChannelConfig(ADC1, channel, 1, ADC_SampleTime_239Cycles5);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    return ADC_GetConversionValue(ADC1);
}

/**
  * @brief  获取光照强度估算值。
  * @retval Lux光照值，显示上限钳制为999。
  * @note   先做10次ADC平均，再根据分压电阻经验公式估算Lux。
  */
uint16_t LDR_GetValue(void)
{
    uint16_t adc_sum = 0;
    float voltage = 0;
    float R = 0;
    uint16_t Lux = 0;

    for(uint8_t i = 0; i < 10; i++)
    {
        adc_sum += LDR_ADC_Value(4);
    }
    adc_sum /= 10;

    voltage = adc_sum;
    voltage  = voltage / 4096 * 3.3f;

    R = voltage / (3.3f - voltage) * 10000;

    Lux = 40000 * pow(R, -0.6021);

    if (Lux > 1000)
    {
        Lux = 1000;
    }
    return Lux;
}

uint16_t ldr;  /* 当前光照强度 */

/**
  * @brief  周期采集光照值，并更新全局变量ldr。
  */
void GET_LDR(void)
{
	ldr = LDR_GetValue();
}
