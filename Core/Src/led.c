#include "led.h"

void LED_On(port GPIOX, LED pin)
{
    HAL_GPIO_WritePin(GPIOX, pin, GPIO_PIN_RESET);
}

void LED_Off(port GPIOX, LED pin)
{
    HAL_GPIO_WritePin(GPIOX, pin, GPIO_PIN_SET);
}
void LED_Toggle(port GPIOX, LED pin)
{
    HAL_GPIO_TogglePin(GPIOX, pin);
}
