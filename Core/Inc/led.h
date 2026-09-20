#ifndef __LED_H
#define __LED_H

#include "gpio.h"
#define LED uint16_t
#define port GPIO_TypeDef *

void LED_On(port GPIOX, LED pin);
void LED_Off(port GPIOX, LED pin);
void LED_Toggle(port GPIOX, LED pin);

#endif
