#include "freertos_demo.h"

#include "led.h"
#include "key.h"
#include "usart.h"
#include "xpt2046.h"
#include "lvgl.h"
#include "lv_port_indev.h"
#include "lv_port_lcd_stm32.h"

#define START_TASK_STACK 256
#define START_TASK_PRIORITY 5
TaskHandle_t start_task_handle;
void start_task(void *pvParameters);

#define TASK1_STACK 256
#define TASK1_PRIORITY 4
TaskHandle_t task1_handle;
void task1(void *pvParameters);

#define TASK2_STACK 256
#define TASK2_PRIORITY 2
TaskHandle_t task2_handle;
void task2(void *pvParameters);

#define TASK3_STACK 256
#define TASK3_PRIORITY 3
TaskHandle_t task3_handle;
void task3(void *pvParameters);

#define LVGL_STACK 2048
#define LVGL_PRIORITY 1
TaskHandle_t lvgl_task_handle;
void lvgl_task(void *pvParameters);

void freertos_start(void)
{
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

    xTaskCreate((TaskFunction_t)task1,
                (char *)"task1",
                (configSTACK_DEPTH_TYPE)TASK1_STACK,
                (void *)NULL,
                (UBaseType_t)TASK1_PRIORITY,
                (TaskHandle_t *)&task1_handle);

    xTaskCreate((TaskFunction_t)task2,
                (char *)"task2",
                (configSTACK_DEPTH_TYPE)TASK2_STACK,
                (void *)NULL,
                (UBaseType_t)TASK2_PRIORITY,
                (TaskHandle_t *)&task2_handle);

    xTaskCreate((TaskFunction_t)task3,
                (char *)"task3",
                (configSTACK_DEPTH_TYPE)TASK3_STACK,
                (void *)NULL,
                (UBaseType_t)TASK3_PRIORITY,
                (TaskHandle_t *)&task3_handle);

    xTaskCreate((TaskFunction_t)lvgl_task,
                (char *)"lvgl_task",
                (configSTACK_DEPTH_TYPE)LVGL_STACK,
                (void *)NULL,
                (UBaseType_t)LVGL_PRIORITY,
                (TaskHandle_t *)&lvgl_task_handle);

    vTaskDelete(NULL);
    taskEXIT_CRITICAL();
}

void task1(void *pvParameters)
{
    while (1)
    {
        LED_On(GPIOC, GPIO_PIN_13);
        printf_dma("Task 1 is running\n");
        vTaskDelay(1000);
    }
}

void task2(void *pvParameters)
{
    while (1)
    {
        LED_Off(GPIOC, GPIO_PIN_13);
        printf_dma("Task 2 is running\n");
        vTaskDelay(1000);
    }
}

void task3(void *pvParameters)
{
    while (1)
    {
        if (Key_GetEvent() == KEY_1)
        {
            if (task1_handle != NULL)
                vTaskDelete(task1_handle),
                    task1_handle = NULL;
            printf_dma("Task 1 is deleted\n");
        }
        if (Key_GetEvent() == KEY_2)
        {
            if (task2_handle != NULL)
                vTaskDelete(task2_handle),
                    task2_handle = NULL;
            printf_dma("Task 2 is deleted\n");
        }
        printf_dma("Task 3 is running\n");
        vTaskDelay(1000);
    }
}

void lvgl_task(void *pvParameters)
{
    lv_init();
    lv_tick_set_cb(xTaskGetTickCount);
    lv_port_disp_init();
    lv_port_indev_init();
    lv_obj_t *label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "Hello FreeRTOS + LVGL");
    lv_obj_center(label);
    //  printf_dma("LVGL task is ok\n");
    while (1)
    {
        // printf_dma("LVGL task is running\n");
        uint32_t t1 = xTaskGetTickCount();
        lv_task_handler();
        printf_dma("LVGL handler: %lu ms\r\n", t1);
        vTaskDelay(5); // 5ms Ë¢ÐÂÒ»´Î
    }
}
