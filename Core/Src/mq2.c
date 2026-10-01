#include "mq2.h"

uint16_t MQ2_ReadADC(void)
{

    uint32_t sum = 0;
    for (uint8_t i = 0; i < 10; i++)
    {
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, 100);
        sum += HAL_ADC_GetValue(&hadc1);
        HAL_ADC_Stop(&hadc1);
    }
    return sum / 10;
}

float MQ2_Getppm(void)
{
    float vrl = MQ2_ReadADC() * 3.3f / 4095.0f; // ADC -> 电压
    if (vrl < 0.01f)
        vrl = 0.01f;

    float rs = (5.0f - vrl) * 10.0f / vrl; // 算 Rs，RL 取 10k
    float r0 = 5.0f;                       // 经验值，先在空气里试

    return 574.25f * powf(rs / r0, -2.222f); // LPG 曲线
}
