#include "sht30.h"

/**
 * @brief  获取sht30传感器数据
 * @retval 无
 */
Sht30Data Sht30_Getdata(void)
{
    uint8_t data[2] = {0x2C, 0x06};  // 发送测量命令
    uint8_t rxData[6] = {0};         // 接收数据缓冲区
    Sht30Data result = {0.0f, 0.0f}; // 存储温湿度数据的结构体
    // 发送测量命令
    if (HAL_I2C_Master_Transmit(&hi2c1, SHT30_ADDRESS, data, 2, 100) != HAL_OK)
    {
        return result;
    }

    HAL_Delay(20);

    if (HAL_I2C_Master_Receive(&hi2c1, SHT30_ADDRESS, rxData, 6, 100) != HAL_OK)
    {
        return result;
    }

    uint16_t raw_temp = (rxData[0] << 8) | rxData[1];
    result.temp = -45.0f + 175.0f * ((float)raw_temp / 65535.0f);

    uint16_t raw_humi = (rxData[3] << 8) | rxData[4];
    result.humi = 100.0f * ((float)raw_humi / 65535.0f);

    return result;
}

/**
 * @brief  重置sht30传感器
 * @retval 无
 */
uint8_t Sht30_Reset(void)
{
    uint8_t data[2] = {0x30, 0xA2}; // 发送重置命令
    if (HAL_I2C_Master_Transmit(&hi2c1, SHT30_ADDRESS, data, 2, 100) != HAL_OK)
    {
        return 1; // 重置失败
    }
    HAL_Delay(10); // 等待传感器重置完成
    return 0;      // 重置成功
}
