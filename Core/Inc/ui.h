#ifndef __UI_H
#define __UI_H

#include "lvgl.h"
#include "freertos_demo.h"

/* 初始化 UI（在 lvgl_task 里调用） */
void UI_Init(void);

/* 数据更新接口（被 esp_task / sensor_task 调用） */
void UI_UpdateSensor(float temp, float humi, float smoke);
void UI_UpdateActuator(uint8_t led, uint8_t buzzer);
void UI_UpdateMode(uint8_t mode);
void UI_UpdateNetwork(uint8_t wifi, uint8_t mqtt);

/* 消息队列（可选，用于线程安全更新） */
extern QueueHandle_t ui_queue;

#endif
