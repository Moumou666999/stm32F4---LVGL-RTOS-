#include "sht30.h"

static void Delay_us(uint8_t us)
{
    uint32_t start = __HAL_TIM_GET_COUNTER(&htim6);
    while ((__HAL_TIM_GET_COUNTER(&htim6) - start) < us)
        ;
}

static void I2c_W_SCL(uint8_t value)
{
    if (value)
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
    Delay_us(10);
}

static void I2c_W_SDA(uint8_t value)
{
    if (value)
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
    Delay_us(10);
}

static uint8_t I2c_R_SDA(void)
{
    Delay_us(10);
    return HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7);
}

static void i2c_write_byte(uint8_t data)
{
    for (uint8_t i = 0; i < 8; i++)
    {
        I2c_W_SDA((data & 0x80) ? 1 : 0);
        I2c_W_SCL(1);
        I2c_W_SCL(0);
        data <<= 1;
    }
}

static uint8_t i2c_read_byte(void)
{
    uint8_t data = 0;
    I2c_W_SDA(1); // 释放SDA线
    for (uint8_t i = 0; i < 8; i++)
    {
        data <<= 1;
        I2c_W_SCL(1);
        if (I2c_R_SDA())
            data |= 0x01;
        I2c_W_SCL(0);
    }
    return data;
}

static void i2c_send_ack(uint8_t ack)
{
    I2c_W_SDA(ack ? 0 : 1); // 发送ACK或NACK
    I2c_W_SCL(1);
    I2c_W_SCL(0);
    I2c_W_SDA(1); // 释放SDA线
}

static uint8_t i2c_receive_ack(void)
{
    I2c_W_SDA(1); // 释放SDA线
    I2c_W_SCL(1);
    uint8_t ack = I2c_R_SDA();
    I2c_W_SCL(0);
    return ack;
}

static void i2c_start(void)
{
    I2c_W_SDA(1);
    I2c_W_SCL(1);
    I2c_W_SDA(0);
    I2c_W_SCL(0);
}

static void i2c_stop(void)
{
    I2c_W_SDA(0);
    I2c_W_SCL(1);
    I2c_W_SDA(1);
}

static uint8_t i2c_CRC8(const uint8_t *Data, uint8_t len)
{
    uint8_t crc = 0xFF;
    while (len--)
    {
        crc ^= *Data++;
        for (uint8_t i = 0; i < 8; i++)
        {
            if (crc & 0x80)
                crc = (crc << 1) ^ 0x31;
            else
                crc <<= 1;
        }
    }
    return crc;
}

/**
 * @brief  获取sht30传感器数据
 * @retval 无
 */
Sht30Data Sht30_Getdata(void)
{
    // uint8_t data[2] = {0x2C, 0x06};  // 发送测量命令
    uint8_t rxData[6] = {0};         // 接收数据缓冲区
    Sht30Data result = {0.0f, 0.0f}; // 存储温湿度数据的结构体
    // uint8_t ack = 1;
    //  发送测量命令
    i2c_start();
    i2c_write_byte(0x44 << 1); // SHT30 I2C地址
    i2c_receive_ack();         // 接收ACK
    i2c_write_byte(0x2c);
    i2c_receive_ack();
    i2c_write_byte(0x06); // 测量命令
    i2c_receive_ack();

    HAL_Delay(20);

    i2c_start();
    i2c_write_byte(0x44 << 1 | 0x01); // SHT30 I2C地址（读取模式）
    i2c_receive_ack();          // 接收ACK

    for (uint8_t i = 0; i < 6; i++)
    {
        rxData[i] = i2c_read_byte();
        if (i < 5)
            i2c_send_ack(1); // 发送ACK
        else
            i2c_send_ack(0); // 最后一个字节发送NACK
    }

    i2c_stop();

    if (i2c_CRC8(&rxData[0], 2) != rxData[2])
        return result; // 温度 CRC 错误
    if (i2c_CRC8(&rxData[3], 2) != rxData[5])
        return result; // 湿度 CRC 错误

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
    i2c_start();
    i2c_write_byte(0x44 << 1); // SHT30 I2C地址
    i2c_write_byte(data[0]);   // 发送重置命令
    i2c_write_byte(data[1]);
    i2c_stop();
    HAL_Delay(10); // 等待传感器重置完成
    return 0;      // 重置成功
}
