#ifndef __XPT2046_H
#define __XPT2046_H

#include "main.h"
#include "lvgl.h"

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
