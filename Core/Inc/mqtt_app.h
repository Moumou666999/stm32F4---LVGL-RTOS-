#ifndef __MQTT_APP_H
#define __MQTT_APP_H

#include "stdint.h"

/* 上报温湿度烟雾 */
uint8_t MQTT_ReportSensor(float temp, float humi, float smoke);

/* 上报执行器状态 */
uint8_t MQTT_ReportActuator(uint8_t led, uint8_t fan, uint8_t buzzer);

/* 上报工作模式 */
uint8_t MQTT_ReportMode(uint8_t mode);

/* 上报固件版本 */
uint8_t MQTT_ReportVersion(const char *version);

/* 解析下发消息*/
void MQTT_ProcessCloudCmd(const char *urc_line);

void MQTT_ReplyCloudCmd(const char *id);

#endif // __MQTT_APP_H
