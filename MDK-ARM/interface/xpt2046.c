#include "xpt2046.h"
#include "spi.h"
#include <stdint.h>

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

static void Touch_CS_Low(void)
{
    HAL_GPIO_WritePin(TOUCH_CS_PORT, TOUCH_CS_PIN, GPIO_PIN_RESET);
}

static void Touch_CS_High(void)
{
    HAL_GPIO_WritePin(TOUCH_CS_PORT, TOUCH_CS_PIN, GPIO_PIN_SET);
}

static uint16_t XPT2046_ReadChannel(uint8_t command)
{
    uint8_t tx[3] = {command, 0x00, 0x00};
    uint8_t rx[3] = {0};

    Touch_CS_Low();

    if (HAL_SPI_TransmitReceive(&hspi2, tx, rx, 3, 100) != HAL_OK)
    {
        Touch_CS_High();
        return 0;
    }

    Touch_CS_High();

    /* XPT2046 result is 12-bit, aligned in the received 16-bit stream. */
    return (uint16_t)((((uint16_t)rx[1] << 8) | rx[2]) >> 3);
}

uint16_t Touchpad_ReadRawX(void)
{
    return XPT2046_ReadChannel(XPT2046_CMD_X);
}

uint16_t Touchpad_ReadRawY(void)
{
    return XPT2046_ReadChannel(XPT2046_CMD_Y);
}

uint8_t Touchpad_IsPressed(void)
{
    /* T_IRQ / PENIRQ is low while the panel is being touched. */
    return (HAL_GPIO_ReadPin(TOUCH_IRQ_PORT, TOUCH_IRQ_PIN) == GPIO_PIN_RESET);
}

static int32_t MapValue(int32_t value,
                        int32_t in_min,
                        int32_t in_max,
                        int32_t out_min,
                        int32_t out_max)
{
    if (in_max == in_min)
        return out_min;

    return (value - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static int32_t ClampValue(int32_t value, int32_t min, int32_t max)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

static void touchpad_get_xy(int32_t *x, int32_t *y)
{
    uint16_t raw_x = Touchpad_ReadRawX();
    uint16_t raw_y = Touchpad_ReadRawY();

#if TOUCH_SWAP_XY
    *x = MapValue(raw_y, RAW_Y_MIN, RAW_Y_MAX, 0, TOUCH_SCREEN_WIDTH - 1);
    *y = MapValue(raw_x, RAW_X_MIN, RAW_X_MAX, 0, TOUCH_SCREEN_HEIGHT - 1);
#else
    *x = MapValue(raw_x, RAW_X_MIN, RAW_X_MAX, 0, TOUCH_SCREEN_WIDTH - 1);
    *y = MapValue(raw_y, RAW_Y_MIN, RAW_Y_MAX, 0, TOUCH_SCREEN_HEIGHT - 1);
#endif

#if TOUCH_INVERT_X
    *x = (TOUCH_SCREEN_WIDTH - 1) - *x;
#endif

#if TOUCH_INVERT_Y
    *y = (TOUCH_SCREEN_HEIGHT - 1) - *y;
#endif

    *x = ClampValue(*x, 0, TOUCH_SCREEN_WIDTH - 1);
    *y = ClampValue(*y, 0, TOUCH_SCREEN_HEIGHT - 1);
}

static void touchpad_read(lv_indev_t *indev_drv, lv_indev_data_t *data)
{
    static int32_t last_x = 0;
    static int32_t last_y = 0;

    (void)indev_drv;

    if (Touchpad_IsPressed())
    {
        touchpad_get_xy(&last_x, &last_y);
        data->state = LV_INDEV_STATE_PRESSED;
    }
    else
    {
        data->state = LV_INDEV_STATE_RELEASED;
    }

    data->point.x = last_x;
    data->point.y = last_y;
}

void Touchpad_Init(void)
{
    /* T_CS is active-low; idle must be high. */
    Touch_CS_High();

    // /* LVGL 9.5 input-device API. */
    // lv_indev_t *indev = lv_indev_create();
    // lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    // lv_indev_set_read_cb(indev, touchpad_read);
}
