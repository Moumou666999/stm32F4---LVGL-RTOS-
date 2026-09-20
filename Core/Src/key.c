#include "key.h"

#define KEY_QUEUE_SIZE 8

static volatile KeyEvent_t key_queue[KEY_QUEUE_SIZE];

static volatile uint8_t key_write = 0;
static volatile uint8_t key_read = 0;

KeyEvent_t Key_GetEvent(void)
{
    KeyEvent_t event = KEY_NONE;

    if (key_read != key_write)
    {
        event = key_queue[key_read];
        key_read = (key_read + 1) % KEY_QUEUE_SIZE;
    }

    return event;
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    KeyEvent_t event = KEY_NONE;

    /* 判断是哪一个按键 */
    if (GPIO_Pin == KEY1_Pin)
    {
        HAL_Delay(10); // 消抖延时
        if (HAL_GPIO_ReadPin(KEY1_GPIO_Port, KEY1_Pin) == GPIO_PIN_RESET)
        {
            event = KEY_1;
        }
    }
    else if (GPIO_Pin == KEY2_Pin)
    {
        HAL_Delay(10); // 消抖延时
        if (HAL_GPIO_ReadPin(KEY2_GPIO_Port, KEY2_Pin) == GPIO_PIN_RESET)
        {
            event = KEY_2;
        }
    }

    /* 有效按键事件 */
    if (event != KEY_NONE)
    {
        uint8_t next = (key_write + 1) % KEY_QUEUE_SIZE;

        if (next != key_read)
        {
            key_queue[key_write] = event;

            key_write = next;
        }
    }
}
