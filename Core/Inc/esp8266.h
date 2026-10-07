#ifndef __ESP8266_H
#define __ESP8266_H

#include <stdint.h>

/* ========== 连接参数（可外部定义，也可头文件定义） ========== */
#define ESP_WIFI_SSID "CMCC-1102"
#define ESP_WIFI_PWD "88888888"

#define ONENET_PRO_ID "086S4IA96T"
#define ONENET_DEV_NAME "1"
#define ONENET_MQTT_HOST "mqtts.heclouds.com"
#define ONENET_MQTT_PORT 1883
#define ONENET_CLIENT_ID "1"
#define ONENET_USERNAME "086S4IA96T"
#define ONENET_PASSWORD "version=2018-10-31&res=products%2F086S4IA96T%2Fdevices%2F1&et=1822799447&method=sha1&sign=G2rdajtpdjFm9kc2vq73JJTi5Tg%3D"

/* ========== 状态枚举 ========== */
typedef enum
{
    ESP_STATE_IDLE = 0,
    ESP_STATE_WIFI_CONNECTING,
    ESP_STATE_WIFI_OK,
    ESP_STATE_MQTT_CONNECTING,
    ESP_STATE_MQTT_OK,
    ESP_STATE_ERROR,
} ESP_State_t;

/* ========== 初始化接口 ========== */
uint8_t ESP_Init(void); // 完整初始化：AT → 关回显
uint8_t ESP_ConnectWiFi(const char *ssid, const char *pwd);
uint8_t ESP_ConnectOneNET(void);   // MQTT 用户配置 + 连接
uint8_t ESP_SubscribeTopics(void); // 订阅 post/reply 和 property/set
uint8_t ESP_Reconnect(void);       // 断线重连

/* ========== 数据收发接口 ========== */
uint8_t ESP_Publish(const char *topic, const char *payload);
uint8_t ESP_SendCmd(const char *cmd, const char *expect, uint32_t timeout_ms);
uint8_t ESP_SendCmdOK(const char *cmd, uint32_t timeout_ms);

/* ========== 状态查询 ========== */
ESP_State_t ESP_GetState(void);
uint8_t ESP_IsWiFiOK(void);
uint8_t ESP_IsMQTTOK(void);

#endif
