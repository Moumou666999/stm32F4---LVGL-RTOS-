#include "mqtt_app.h"
#include "esp8266.h"
#include "freertos_demo.h" // 为了拿 g_sys 和 g_mutex
#include "usart.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include <stdio.h>
#include <string.h>

/* 上报主题 */
#define TOPIC_POST "$sys/086S4IA96T/1/thing/property/post"

/* 属性下发回复主题 */
#define TOPIC_SET_REPLY "$sys/086S4IA96T/1/thing/property/set_reply"

/* 公共的 ID，每次上报递增 */
static uint32_t mqtt_msg_id = 0;

/* 上报温湿度烟雾 */
uint8_t MQTT_ReportSensor(float temp, float humi, float smoke)
{
    static char payload[256];

    snprintf(payload, sizeof(payload),
             "{\\\"id\\\":\\\"%lu\\\"\\,\\\"version\\\":\\\"1.0\\\"\\,\\\"params\\\":{"
             "\\\"temp\\\":{\\\"value\\\":%.1f}\\,"
             "\\\"humi\\\":{\\\"value\\\":%.1f}\\,"
             "\\\"smoke\\\":{\\\"value\\\":%.2f}"
             "}}",
             (unsigned long)(mqtt_msg_id++), temp, humi, smoke);

    return ESP_Publish(TOPIC_POST, payload);
}

/* 上报执行器状态 */
uint8_t MQTT_ReportActuator(uint8_t led, uint8_t fan, uint8_t buzzer)
{
    static char payload[256];

    snprintf(payload, sizeof(payload),
             "{\\\"id\\\":\\\"%lu\\\"\\,\\\"version\\\":\\\"1.0\\\"\\,\\\"params\\\":{"
             "\\\"led\\\":{\\\"value\\\":%s}\\,"
             "\\\"fan\\\":{\\\"value\\\":%s}\\,"
             "\\\"buzzer\\\":{\\\"value\\\":%s}"
             "}}",
             (unsigned long)(mqtt_msg_id++),
             led ? "true" : "false",
             fan ? "true" : "false",
             buzzer ? "true" : "false");

    return ESP_Publish(TOPIC_POST, payload);
}

/* 上报工作模式 */
uint8_t MQTT_ReportMode(uint8_t mode)
{
    static char payload[128];

    snprintf(payload, sizeof(payload),
             "{\\\"id\\\":\\\"%lu\\\"\\,\\\"version\\\":\\\"1.0\\\"\\,\\\"params\\\":{"
             "\\\"mode\\\":{\\\"value\\\":\\\"%s\\\"}"
             "}}",
             (unsigned long)(mqtt_msg_id++),
             mode ? "manual" : "auto");

    return ESP_Publish(TOPIC_POST, payload);
}

/* 上报固件版本 */
uint8_t MQTT_ReportVersion(const char *version)
{
    static char payload[128];

    snprintf(payload, sizeof(payload),
             "{\\\"id\\\":\\\"%lu\\\"\\,\\\"version\\\":\\\"1.0\\\"\\,\\\"params\\\":{"
             "\\\"version\\\":{\\\"value\\\":\\\"%s\\\"}"
             "}}",
             (unsigned long)(mqtt_msg_id++), version);

    return ESP_Publish(TOPIC_POST, payload);
}

/* ============ 解析云端属性下发指令 ============ */
void MQTT_ProcessCloudCmd(const char *urc_line)
{
    /* 1. 必须是属性下发消息 */
    if (strstr(urc_line, "+MQTTSUBRECV") == NULL)
        return;
    if (strstr(urc_line, "property/set") == NULL)
        return;

    /* 2. 找到 JSON 起始位置 */
    const char *payload = strchr(urc_line, '{');
    if (payload == NULL)
        return;

    printf_uart("--- Cloud CMD ---\r\n");
    printf_uart("JSON: %s\r\n", payload);

    /* 3. 提取 id */
    char id[16] = "0";
    const char *p = strstr(payload, "\"id\":\"");
    if (p)
    {
        p += 6;
        const char *q = strchr(p, '\"');
        if (q && (q - p) < (int)sizeof(id))
        {
            memcpy(id, p, q - p);
            id[q - p] = '\0';
        }
    }

    /* 4. 提取字段（按 OneNET 的嵌套格式匹配） */
    int led_val = -1;
    int fan_val = -1;
    int buzzer_val = -1;
    int mode_val = -1;

    /* 匹配 "led":{"value":true 或 false */
    if (strstr(payload, "\"led\":true") || strstr(payload, "\"led\":{\"value\":true"))
        led_val = 1;
    else if (strstr(payload, "\"led\":false") || strstr(payload, "\"led\":{\"value\":false"))
        led_val = 0;

    if (strstr(payload, "\"fan\":true") || strstr(payload, "\"fan\":{\"value\":true"))
        fan_val = 1;
    else if (strstr(payload, "\"fan\":false") || strstr(payload, "\"fan\":{\"value\":false"))
        fan_val = 0;

    if (strstr(payload, "\"buzzer\":true") || strstr(payload, "\"buzzer\":{\"value\":true"))
        buzzer_val = 1;
    else if (strstr(payload, "\"buzzer\":false") || strstr(payload, "\"buzzer\":{\"value\":false"))
        buzzer_val = 0;

    /* mode 是枚举字符串 */
    if (strstr(payload, "\"mode\":{\"value\":\"manual\"") || strstr(payload, "\"mode\":\"manual\""))
        mode_val = 1;
    else if (strstr(payload, "\"mode\":{\"value\":\"auto\"") || strstr(payload, "\"mode\":\"auto\""))
        mode_val = 0;

    printf_uart("id=%s led=%d fan=%d buzzer=%d mode=%d\r\n", id, led_val, fan_val, buzzer_val, mode_val);

    /* 5. 更新 g_sys（加锁） */
    if (xSemaphoreTake(g_mutex, portMAX_DELAY) == pdTRUE)
    {
        if (led_val >= 0)
            g_sys.led = led_val;
        if (fan_val >= 0)
            g_sys.fan = fan_val;
        if (buzzer_val >= 0)
            g_sys.buzzer = buzzer_val;
        if (mode_val >= 0)
            g_sys.mode = mode_val;
        xSemaphoreGive(g_mutex);
    }

    /* 6. 回复云端 */
    MQTT_ReplyCloudCmd(id);
}

/* ============ 回复云端 ============ */
void MQTT_ReplyCloudCmd(const char *id)
{
    static char payload[128];

    snprintf(payload, sizeof(payload),
             "{\\\"id\\\":\\\"%s\\\"\\,\\\"code\\\":200\\,\\\"msg\\\":\\\"success\\\"}",
             id);

    ESP_Publish(TOPIC_SET_REPLY, payload);
}
