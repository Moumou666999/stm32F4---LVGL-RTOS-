#include "esp8266.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdio.h>

#define AT_RX_ACC_SIZE 512
static uint8_t at_rx_acc[AT_RX_ACC_SIZE]; // AT 指令接收累加区
static uint16_t at_rx_acc_len = 0;        // AT 指令接收累加长度

/* 状态 */
// static ESP_State_t esp_state = ESP_STATE_IDLE;
// static uint8_t wifi_ok = 0;
// static uint8_t mqtt_ok = 0;

/* 重连计数 */
// static uint32_t wifi_retry = 0;
// static uint32_t mqtt_retry = 0;

/* AT 指令锁（防止多任务同时发） */
static SemaphoreHandle_t at_mutex = NULL;

static void at_send(const char *cmd)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)cmd, strlen(cmd), 500);
    printf_uart(">> %s\r\n", cmd);
}

static uint8_t at_wait_line(const char *expect, uint32_t timeout_ms)
{
    uint32_t start_tick = xTaskGetTickCount();

    at_rx_acc_len = 0; // 清空接收累加区
    at_rx_acc[0] = '\0';

    while (1)
    {
        /* 1. 检查超时 */
        uint32_t elapsed = xTaskGetTickCount() - start_tick;
        if (elapsed >= timeout_ms)
        {
            printf_uart("<< [timeout: expect '%s']\r\n", expect);
            return 0;
        }

        /* 2. 剩余时间作为等待上限 */
        uint32_t remain = timeout_ms - elapsed;
        TickType_t wait_ticks = pdMS_TO_TICKS(remain);
        if (wait_ticks == 0)
            wait_ticks = 1;

        /* 3. 等 ESP 串口数据到达的通知（由 esp_task 或回调给出） */
        if (ulTaskNotifyTake(pdTRUE, wait_ticks) == 0)
        {
            /* 超时没收到数据，继续循环让上面的超时判断处理 */

            continue;
        }

        /* 4. 从接收缓冲读数据 */
        uint8_t tmp[256];
        uint16_t len = ESP_Uart_Read(tmp, sizeof(tmp) - 1);

        if (len == 0)
        {
            continue; /* 空通知，忽略 */
        }
        tmp[len] = '\0';

        /* 5. 打印收到的数据（<< 表示收到） */
        printf_uart("<< %s\r\n", tmp);

        /* 6. 追加到累积缓冲（处理粘包/分包） */
        if (at_rx_acc_len + len < AT_RX_ACC_SIZE - 1)
        {
            memcpy(at_rx_acc + at_rx_acc_len, tmp, len);
            at_rx_acc_len += len;
            at_rx_acc[at_rx_acc_len] = '\0';
        }

        if (strstr((char *)at_rx_acc, "+MQTTSUBRECV") != NULL ||
            strstr((char *)at_rx_acc, "busy") != NULL ||
            strstr((char *)at_rx_acc, "+MQTTDISCONNECTED") != NULL)
        {
            /* 把这些 URC 打印出来，方便调试 */
            printf_uart("--- skip URC ---\r\n");
            /* 清空累积，准备下一批数据 */
            at_rx_acc_len = 0;
            at_rx_acc[0] = '\0';
            continue;
        }

        /* 7. 判断累积缓冲里是否出现期望的字符串 */
        if (strstr(at_rx_acc, expect) != NULL)
        {
            return 1; /* 找到了 */
        }
    }
}
uint8_t ESP_Init(void) // 完整初始化：AT → 关回显
{
    vTaskDelay(pdMS_TO_TICKS(2000)); // 等 ESP8266 上电稳定

    // /*复位ESP8266 */
    // at_send("AT+RESTORE\r\n");
    // vTaskDelay(pdMS_TO_TICKS(6000)); // 等待复位完成
    // ESP_Uart_clear();                // 清空接收缓冲，避免复位信息干扰后续判断

    /* 握手：确认 ESP 在线 */
    if (!ESP_SendCmd("AT\r\n", "OK", 2000))
    {
        printf_uart("ESP: AT fail\r\n");
        return 0;
    }
    printf_uart("ESP: AT OK\r\n");

    // if (!ESP_SendCmd("AT\r\n", "OK", 3000))
    // {
    //     printf_uart("ESP: AT fail\r\n");
    //     return 0;
    // }
    // printf_uart("ESP: AT OK\r\n");

    /* 关闭回显（避免干扰响应判断） */
    if (!ESP_SendCmd("ATE0\r\n", "OK", 2000))
    {
        printf_uart("ESP: ATE0 fail\r\n");
        return 0;
    }
    printf_uart("ESP: echo off\r\n");

    /* 设置 Station 模式 */
    if (!ESP_SendCmd("AT+CWMODE=1\r\n", "OK", 2000))
    {
        printf_uart("ESP: CWMODE fail\r\n");
        return 0;
    }
    printf_uart("ESP: station mode\r\n");

    return 1;
}

uint8_t ESP_ConnectWiFi(const char *ssid, const char *pwd)
{
    uint8_t cmd[128];
    uint8_t retry;

    for (retry = 0; retry < 3; retry++)
    {
        sprintf((char *)cmd, "AT+CWJAP=\"%s\",\"%s\"\r\n", ssid, pwd);
        printf_uart("WIFI Connecting try %d\r\n", retry + 1);

        if (ESP_SendCmd(cmd, "OK", 5000))
        {
            printf_uart("WIFI Connected\r\n");
            return 1;
        }

        printf_uart("WIFI Connect failed\r\n");
        vTaskDelay(pdMS_TO_TICKS(2000)); // 等待 2 秒后重试
    }
    printf_uart("WIFI Connect failed after 3 attempts\r\n");
    return 0;
}

uint8_t ESP_ConnectOneNET(void) // MQTT 用户配置 + 连接
{
    uint8_t cmd[512];

    sprintf((char *)cmd, "AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n", ONENET_CLIENT_ID, ONENET_USERNAME, ONENET_PASSWORD);
    if (!ESP_SendCmd(cmd, "OK", 5000))
    {
        printf_uart("MQTT User Config failed\r\n");
        return 0;
    }
    printf_uart("MQTT User Config OK\r\n");
    // vTaskDelay(pdMS_TO_TICKS(500));

    sprintf((char *)cmd, "AT+MQTTCONN=0,\"%s\",%d,1\r\n", ONENET_MQTT_HOST, ONENET_MQTT_PORT);
    if (!ESP_SendCmd(cmd, "OK", 5000))
    {
        printf_uart("MQTT Connect failed\r\n");
        return 0;
    }
    printf_uart("MQTT Connect OK\r\n");
    return 1;
}

uint8_t ESP_SubscribeTopics(void) // 订阅 post/reply 和 property/set
{
    char cmd[256];

    sprintf(cmd, "AT+MQTTSUB=0,\"$sys/%s/%s/thing/property/post/reply\",0\r\n", ONENET_PRO_ID, ONENET_DEV_NAME);
    if (!ESP_SendCmd(cmd, "OK", 5000))
    {
        printf_uart("Sub post/reply failed\r\n");
        return 0;
    }
    printf_uart("MQTT Subscribe post/reply OK\r\n");
    vTaskDelay(pdMS_TO_TICKS(500)); // 等待 0.5 秒，确保订阅生效
    sprintf(cmd, "AT+MQTTSUB=0,\"$sys/%s/%s/thing/property/set\",0\r\n", ONENET_PRO_ID, ONENET_DEV_NAME);
    if (!ESP_SendCmd(cmd, "OK", 5000))
    {
        printf_uart("MQTT Subscribe property/set failed\r\n");
        return 0;
    }
    printf_uart("MQTT Subscribe service/post/reply OK\r\n");
    return 1;
}

uint8_t ESP_Reconnect(void); // 断线重连

/* ========== 数据收发接口 ========== */
uint8_t ESP_Publish(const char *topic, const char *payload)
{
    static char cmd[512];

    int len = snprintf(cmd, sizeof(cmd), "AT+MQTTPUB=0,\"%s\",\"%s\",0,0\r\n", topic, payload);
    if (len < 0 || len >= sizeof(cmd))
    {
        printf_uart("MQTT Publish: payload too long\r\n");
        return 0;
    }

    return ESP_SendCmd(cmd, "OK", 5000);
}

uint8_t ESP_SendCmd(const char *cmd, const char *expect, uint32_t timeout_ms)
{
    if (at_mutex == NULL)
    {
        at_mutex = xSemaphoreCreateMutex();
    }
    if (xSemaphoreTake(at_mutex, pdMS_TO_TICKS(timeout_ms + 500)) != pdTRUE)
    {
        printf_uart("AT mutex timeout\r\n");
        return 0;
    }
    ESP_Uart_clear(); // 清空接收缓冲，避免干扰响应判断
    at_send(cmd);
    uint8_t ok = at_wait_line(expect, timeout_ms);

    xSemaphoreGive(at_mutex);
    return ok;
}

uint8_t ESP_SendCmdOK(const char *cmd, uint32_t timeout_ms);

/* ========== 状态查询 ========== */
ESP_State_t ESP_GetState(void);
uint8_t ESP_IsWiFiOK(void);
uint8_t ESP_IsMQTTOK(void);
