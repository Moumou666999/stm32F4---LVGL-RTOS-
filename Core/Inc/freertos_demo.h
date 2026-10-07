#ifndef __FREERTOS_DEMO_H
#define __FREERTOS_DEMO_H

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
typedef struct
{
    /* 传感器数据 */
    float temp;
    float humi;
    float smoke;

    /* 阈值 */
    uint8_t T_temp;
    uint8_t T_humi;
    uint8_t T_rate;

    /* 执行器状态 */
    uint8_t led;
    uint8_t fan;
    uint8_t buzzer;

    /* 工作模式和版本 */
    uint8_t mode; // 0: auto, 1: manual
    uint8_t version[16];

    /* 网络状态 */
    uint8_t wifi_station;
    uint8_t mqtt_station;

    /* 串口忙碌标志 */
    uint8_t uart_busy;

} SysData_t;

typedef enum
{
    MQTT_MSG_SENSOR,   // 上报温湿度烟雾
    MQTT_MSG_ACTUATOR, // 上报执行器状态
    MQTT_MSG_MODE,     // 上报工作模式
    MQTT_MSG_VERSION,  // 上报固件版本
} MQTT_MsgType_t;

typedef struct
{
    MQTT_MsgType_t type;
    union
    {
        struct
        {
            float temp;
            float humi;
            float smoke;
        } sensor;
        struct
        {
            uint8_t led;
            uint8_t fan;
            uint8_t buzzer;
        } actuator;
        struct
        {
            uint8_t mode;
        } mode;
        struct
        {
            char version[16];
        } version;
    } data;
} MQTT_Msg_t;

extern QueueHandle_t mqtt_pub_queue;

extern SysData_t g_sys;           // 全局状态
extern SemaphoreHandle_t g_mutex; // 保护 g_sys 的互斥锁
extern QueueHandle_t ui_queue;    // UI 更新队列

void freertos_start(void);

#endif
