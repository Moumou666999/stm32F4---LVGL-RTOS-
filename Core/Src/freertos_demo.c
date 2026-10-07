#include "freertos_demo.h"
#include "led.h"
#include "key.h"
#include "usart.h"
#include "xpt2046.h"
#include "lvgl.h"
#include "lv_port_indev.h"
#include "lv_port_lcd_stm32.h"
#include "esp8266.h"
#include "mqtt_app.h"
#include "sht30.h"
#include "mq2.h"
#include "queue.h"
#include "semphr.h"
#include "ui.h"

#define START_TASK_STACK 256
#define START_TASK_PRIORITY 6
TaskHandle_t start_task_handle;
void start_task(void *pvParameters);

#define SENSOR_STACK 512
#define SENSOR_PRIORITY 3
TaskHandle_t sensor_task_handle;
void sensor_task(void *pvParameters);

#define MQTT_PUB_STACK 1024
#define MQTT_PUB_PRIORITY 2
TaskHandle_t mqtt_pub_task_handle;
void mqtt_pub_task(void *pvParameters);

#define TASK3_STACK 256
#define TASK3_PRIORITY 4
TaskHandle_t task3_handle;
void task3(void *pvParameters);

#define LVGL_STACK 4096
#define LVGL_PRIORITY 3
TaskHandle_t lvgl_task_handle;
void lvgl_task(void *pvParameters);

#define ESP_STACK 1024
#define ESP_PRIORITY 4
TaskHandle_t esp_task_handle;
void esp_task(void *pvParameters);

SysData_t g_sys =
    {
        /* 传感器初始值 */
        .temp = 20.0f,
        .humi = 20.0f,
        .smoke = 20.0f,

        /* 阈值初始值 */
        .T_temp = 29,
        .T_humi = 70,
        .T_rate = 20,

        /* 执行器初始状态 */
        .fan = 0,
        .buzzer = 0,
        .led = 0,

        /* 系统模式 */
        .mode = 0, // 默认自动

        /* 网络状态 */
        .wifi_station = 0,
        .mqtt_station = 0,

        .uart_busy = 0,
        .version = "1.00",
};

SemaphoreHandle_t g_mutex = NULL;
// QueueHandle_t ui_queue = NULL;
QueueHandle_t mqtt_pub_queue;

void freertos_start(void)
{
    mqtt_pub_queue = xQueueCreate(10, sizeof(MQTT_Msg_t));
    xTaskCreate((TaskFunction_t)start_task,
                (char *)"start_task",
                (configSTACK_DEPTH_TYPE)START_TASK_STACK,
                (void *)NULL,
                (UBaseType_t)START_TASK_PRIORITY,
                (TaskHandle_t *)&start_task_handle);

    vTaskStartScheduler();
}

void start_task(void *pvParameters)
{
    taskENTER_CRITICAL();

    g_mutex = xSemaphoreCreateMutex();
    // ui_queue = xQueueCreate(10, sizeof(UI_Msg_t));

    xTaskCreate((TaskFunction_t)sensor_task,
                (char *)"sensor_task",
                (configSTACK_DEPTH_TYPE)SENSOR_STACK,
                (void *)NULL,
                (UBaseType_t)SENSOR_PRIORITY,
                (TaskHandle_t *)&sensor_task_handle);

    xTaskCreate((TaskFunction_t)mqtt_pub_task,
                (char *)"mqtt_pub_task",
                (configSTACK_DEPTH_TYPE)MQTT_PUB_STACK,
                (void *)NULL,
                (UBaseType_t)MQTT_PUB_PRIORITY,
                (TaskHandle_t *)&mqtt_pub_task_handle);

    // xTaskCreate((TaskFunction_t)task3,
    //             (char *)"task3",
    //             (configSTACK_DEPTH_TYPE)TASK3_STACK,
    //             (void *)NULL,
    //             (UBaseType_t)TASK3_PRIORITY,
    //             (TaskHandle_t *)&task3_handle);

    xTaskCreate((TaskFunction_t)lvgl_task,
                (char *)"lvgl_task",
                (configSTACK_DEPTH_TYPE)LVGL_STACK,
                (void *)NULL,
                (UBaseType_t)LVGL_PRIORITY,
                (TaskHandle_t *)&lvgl_task_handle);

    xTaskCreate((TaskFunction_t)esp_task,
                (char *)"esp_task",
                (configSTACK_DEPTH_TYPE)ESP_STACK,
                (void *)NULL,
                (UBaseType_t)ESP_PRIORITY,
                (TaskHandle_t *)&esp_task_handle);

    taskEXIT_CRITICAL();
    vTaskDelete(NULL);
}

void sensor_task(void *pvParameters)
{
    Sht30Data sht;

    while (1)
    {
        sht = Sht30_Getdata();
        float smoke = MQ2_Getppm();

        if (xSemaphoreTake(g_mutex, portMAX_DELAY) == pdTRUE)
        {
            /* 2. 改数据 */
            g_sys.temp = sht.temp;
            g_sys.humi = sht.humi;
            g_sys.smoke = smoke;

            /* 3. 放锁 */
            xSemaphoreGive(g_mutex);
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void mqtt_pub_task(void *pvParameters)
{
    SysData_t sht;
    uint32_t last_sensor_report;
    MQTT_Msg_t mqtt_pub;

    while (1)
    {
        if (xSemaphoreTake(g_mutex, portMAX_DELAY) == pdTRUE)
        {
            sht = g_sys;
            xSemaphoreGive(g_mutex);
        }

        if (xTaskGetTickCount() - last_sensor_report >= pdMS_TO_TICKS(5000))
        {
            mqtt_pub.type = MQTT_MSG_SENSOR;
            mqtt_pub.data.sensor.humi = sht.humi;
            mqtt_pub.data.sensor.smoke = sht.smoke;
            mqtt_pub.data.sensor.temp = sht.temp;
            last_sensor_report = xTaskGetTickCount();
            xQueueSend(mqtt_pub_queue, &mqtt_pub, portMAX_DELAY);
        }
    }
}

void task3(void *pvParameters)
{
    while (1)
    {
    }
}

void lvgl_task(void *pvParameters)
{
    lv_init();
    lv_tick_set_cb(xTaskGetTickCount);
    lv_port_disp_init();
    lv_port_indev_init();
    // uint32_t last_check = 0;
    // lv_obj_t *label = lv_label_create(lv_scr_act());
    // lv_label_set_text(label, "LVGL OK");
    // lv_obj_set_style_text_color(label, lv_color_hex(0xFF0000), 0);
    // lv_obj_center(label);

    UI_Init();

    //  printf_uart("LVGL task is ok\n");
    while (1)
    {
        // printf_uart("LVGL task is running\n");
        // if (xTaskGetTickCount() - last_check >= pdMS_TO_TICKS(5000))
        // {
        //     last_check = xTaskGetTickCount();
        //     printf_uart("sensor:%u mqtt:%u lvgl:%u esp:%u\r\n",
        //                 uxTaskGetStackHighWaterMark(sensor_task_handle),
        //                 uxTaskGetStackHighWaterMark(mqtt_pub_task_handle),
        //                 uxTaskGetStackHighWaterMark(NULL), // NULL = 当前任务
        //                 uxTaskGetStackHighWaterMark(esp_task_handle));
        // }
        lv_task_handler();
        vTaskDelay(5); // 5ms 刷新一次
    }
}

void esp_task(void *pvParameters)
{
    esp_task_handle = xTaskGetCurrentTaskHandle();

    ESP_UART_Init();

    MQTT_Msg_t msg;
    uint8_t rx_buf[512];
    /* 1. 初始化 */
    if (!ESP_Init())
    {
        printf_uart("### ESP init FAIL ###\r\n");
        while (1)
            vTaskDelay(pdMS_TO_TICKS(1000));
    }

    /* 2. 连 WiFi */
    if (!ESP_ConnectWiFi(ESP_WIFI_SSID, ESP_WIFI_PWD))
    {
        printf_uart("### WiFi FAIL ###\r\n");
        while (1)
            vTaskDelay(pdMS_TO_TICKS(1000));
    }

    /* 3. 连 OneNET MQTT */
    if (!ESP_ConnectOneNET())
    {
        printf_uart("### MQTT FAIL ###\r\n");
        while (1)
            vTaskDelay(pdMS_TO_TICKS(1000));
    }

    /* 4. 订阅主题 */
    if (!ESP_SubscribeTopics())
    {
        printf_uart("### Subscribe FAIL ###\r\n");
        while (1)
            vTaskDelay(pdMS_TO_TICKS(1000));
    }

    printf_uart("### All ready! ###\r\n");

    while (1)
    {
        /* 1. 处理接收到的 URC */
        if (ESP_Uart_HasData() > 0)
        {
            uint16_t len = ESP_Uart_Read(rx_buf, sizeof(rx_buf) - 1);
            if (len > 0)
            {
                rx_buf[len] = '\0';
                printf_uart("URC: %s\r\n", rx_buf);
                /* 后续解析 +MQTTSUBRECV */
            }
        }

        while (xQueueReceive(mqtt_pub_queue, &msg, 0) == pdTRUE)
        {
            switch (msg.type)
            {
            case MQTT_MSG_SENSOR:
                MQTT_ReportSensor(msg.data.sensor.temp,
                                  msg.data.sensor.humi,
                                  msg.data.sensor.smoke);
                break;
            case MQTT_MSG_ACTUATOR:
                MQTT_ReportActuator(msg.data.actuator.led,
                                    msg.data.actuator.fan,
                                    msg.data.actuator.buzzer);
                break;
            case MQTT_MSG_MODE:
                MQTT_ReportMode(msg.data.mode.mode);
                break;
            default:
                break;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
