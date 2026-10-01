#ifndef __XPT2046_H
#define __XPT2046_H

#include "main.h"
#include "lvgl.h"
#define TOUCH_CS_PORT GPIOC
#define TOUCH_CS_PIN GPIO_PIN_1

#define TOUCH_IRQ_PORT GPIOC
#define TOUCH_IRQ_PIN GPIO_PIN_3

#define XPT2046_CMD_X 0xD0
#define XPT2046_CMD_Y 0x90

#define TOUCH_SCREEN_WIDTH 320
#define TOUCH_SCREEN_HEIGHT 240

/* Initial calibration values. Adjust after testing raw coordinates. */
#define RAW_X_MIN 200
#define RAW_X_MAX 3900
#define RAW_Y_MIN 200
#define RAW_Y_MAX 3900

/* Coordinate orientation. Change to 1 only when testing shows it is needed. */
#define TOUCH_SWAP_XY 0
#define TOUCH_INVERT_X 0
#define TOUCH_INVERT_Y 0

/* MSP2807 / XPT2046
 * LCD: 320 x 240
 * T_CS  -> PC1
 * T_IRQ -> PC3
 * Touch uses SPI2
 * PC3 is polled; EXTI is not required.
 */

/* Initialize and register the LVGL touch input device. */
void Touchpad_Init(void);

/* Optional: raw readings for calibration/debugging. */
uint16_t Touchpad_ReadRawX(void);
uint16_t Touchpad_ReadRawY(void);

/* Optional: current touch state. 1 = pressed, 0 = released. */
uint8_t Touchpad_IsPressed(void);

#endif /* __TOUCHPAD_H */
