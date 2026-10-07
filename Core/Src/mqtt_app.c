#include "mqtt_app.h"
#include "esp8266.h"

/* 上报主题 */
#define TOPIC_POST "$sys/086S4IA96T/1/thing/property/post"

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