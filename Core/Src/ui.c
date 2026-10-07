#include "ui.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>

/* ============ 主页控件句柄 ============ */
static lv_obj_t *label_temp_val;
static lv_obj_t *label_humi_val;
static lv_obj_t *label_smoke_val;
static lv_obj_t *label_mode_val;
static lv_obj_t *label_wifi_status;
static lv_obj_t *label_mqtt_status;

/* ============ 控制页控件句柄 ============ */
static lv_obj_t *sw_led;
static lv_obj_t *sw_buzzer;
static lv_obj_t *label_mode_ctrl;

/* ============ 设置页控件句柄 ============ */
static lv_obj_t *label_T_temp;
static lv_obj_t *label_T_humi;
static lv_obj_t *label_T_smoke;

/* ============ 当前 UI 值（用于 UI 更新判断） ============ */
static float ui_last_temp = -999;
static float ui_last_humi = -999;
static float ui_last_smoke = -999;

/* ============ 创建数据卡片（辅助函数） ============ */
static lv_obj_t *create_card(lv_obj_t *parent, const char *title,
                             lv_color_t color, int16_t x, int16_t y)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, 105, 80);
    lv_obj_set_pos(card, x, y);
    lv_obj_set_style_bg_color(card, color, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_radius(card, 8, 0);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    /* 标题 */
    lv_obj_t *title_label = lv_label_create(card);
    lv_label_set_text(title_label, title);
    lv_obj_set_style_text_color(title_label, lv_color_white(), 0);
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 0);

    return card;
}

/* ============ 创建数值标签（辅助函数） ============ */
static lv_obj_t *create_value_label(lv_obj_t *parent)
{
    lv_obj_t *lbl = lv_label_create(parent);
    lv_label_set_text(lbl, "--");
    lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
    lv_obj_set_style_text_font(lbl, &lv_font_montserrat_14, 0);
    lv_obj_align(lbl, LV_ALIGN_CENTER, 0, 10);
    return lbl;
}

/* ============ 主页 ============ */
static void create_main_page(lv_obj_t *parent)
{
    /* 温度卡片（橙） */
    lv_obj_t *card_temp = create_card(parent, "Temp", lv_color_hex(0xFF5722), 5, 5);
    label_temp_val = create_value_label(card_temp);

    /* 湿度卡片（蓝） */
    lv_obj_t *card_humi = create_card(parent, "Humi", lv_color_hex(0x2196F3), 120, 5);
    label_humi_val = create_value_label(card_humi);

    /* 烟雾卡片（灰） */
    lv_obj_t *card_smoke = create_card(parent, "Smoke", lv_color_hex(0x607D8B), 5, 95);
    label_smoke_val = create_value_label(card_smoke);

    /* 模式卡片（绿） */
    lv_obj_t *card_mode = create_card(parent, "Mode", lv_color_hex(0x4CAF50), 120, 95);
    label_mode_val = create_value_label(card_mode);
    lv_obj_set_style_text_font(label_mode_val, &lv_font_montserrat_14, 0);

    /* 底部网络状态 */
    label_wifi_status = lv_label_create(parent);
    lv_label_set_text(label_wifi_status, "WiFi: --");
    lv_obj_set_style_text_color(label_wifi_status, lv_color_hex(0x9E9E9E), 0);
    lv_obj_align(label_wifi_status, LV_ALIGN_BOTTOM_LEFT, 10, -5);

    label_mqtt_status = lv_label_create(parent);
    lv_label_set_text(label_mqtt_status, "MQTT: --");
    lv_obj_set_style_text_color(label_mqtt_status, lv_color_hex(0x9E9E9E), 0);
    lv_obj_align(label_mqtt_status, LV_ALIGN_BOTTOM_RIGHT, -10, -5);
}

/* ============ 控制页 ============ */
static lv_obj_t *create_control_row(lv_obj_t *parent, const char *name,
                                    int16_t y, lv_obj_t **sw_out)
{
    /* 名称标签 */
    lv_obj_t *lbl = lv_label_create(parent);
    lv_label_set_text(lbl, name);
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 10, y);

    /* 开关 */
    lv_obj_t *sw = lv_switch_create(parent);
    lv_obj_align(sw, LV_ALIGN_TOP_RIGHT, -10, y - 5);
    *sw_out = sw;

    return sw;
}

static void create_control_page(lv_obj_t *parent)
{
    create_control_row(parent, "LED", 20, &sw_led);
    create_control_row(parent, "Buzzer", 70, &sw_buzzer);

    /* 模式显示 */
    label_mode_ctrl = lv_label_create(parent);
    lv_label_set_text(label_mode_ctrl, "Mode: Auto");
    lv_obj_align(label_mode_ctrl, LV_ALIGN_TOP_LEFT, 10, 120);
}

/* ============ 设置页 ============ */
static void create_setting_row(lv_obj_t *parent, const char *name,
                               int16_t y, lv_obj_t **value_out)
{
    /* 名称 */
    lv_obj_t *lbl = lv_label_create(parent);
    lv_label_set_text(lbl, name);
    lv_obj_align(lbl, LV_ALIGN_TOP_LEFT, 10, y);

    /* 数值 */
    lv_obj_t *val = lv_label_create(parent);
    lv_label_set_text(val, "--");
    lv_obj_set_style_text_font(val, &lv_font_montserrat_14, 0);
    lv_obj_align(val, LV_ALIGN_TOP_RIGHT, -10, y);
    *value_out = val;
}

static void create_setting_page(lv_obj_t *parent)
{
    create_setting_row(parent, "T Temp", 20, &label_T_temp);
    create_setting_row(parent, "T Humi", 60, &label_T_humi);
    create_setting_row(parent, "T Smoke", 100, &label_T_smoke);
}

/* ============ 初始化整个 UI ============ */
void UI_Init(void)
{
    /* 创建 TabView */
    lv_obj_t *tabview = lv_tabview_create(lv_scr_act());
    lv_tabview_set_tab_bar_position(tabview, LV_DIR_TOP);
    lv_tabview_set_tab_bar_size(tabview, 30);

    /* 三个页面 */
    lv_obj_t *tab_main = lv_tabview_add_tab(tabview, "Main");
    lv_obj_t *tab_ctrl = lv_tabview_add_tab(tabview, "Ctrl");
    lv_obj_t *tab_set = lv_tabview_add_tab(tabview, "Set");

    /* 设置每个 tab 的样式 */
    lv_obj_set_style_bg_color(tab_main, lv_color_hex(0x212121), 0);
    lv_obj_set_style_bg_color(tab_ctrl, lv_color_hex(0x212121), 0);
    lv_obj_set_style_bg_color(tab_set, lv_color_hex(0x212121), 0);

    /* 创建页面内容 */
    create_main_page(tab_main);
    create_control_page(tab_ctrl);
    create_setting_page(tab_set);
}

/* ============ 更新接口 ============ */

void UI_UpdateSensor(float temp, float humi, float smoke)
{
    char buf[16];

    /* 只在变化时更新，避免频繁刷新 */
    if (fabsf(temp - ui_last_temp) > 0.1f)
    {
        ui_last_temp = temp;
        snprintf(buf, sizeof(buf), "%.1f", temp);
        lv_label_set_text(label_temp_val, buf);
    }
    if (fabsf(humi - ui_last_humi) > 0.1f)
    {
        ui_last_humi = humi;
        snprintf(buf, sizeof(buf), "%.1f", humi);
        lv_label_set_text(label_humi_val, buf);
    }
    if (fabsf(smoke - ui_last_smoke) > 0.01f)
    {
        ui_last_smoke = smoke;
        snprintf(buf, sizeof(buf), "%.2f", smoke);
        lv_label_set_text(label_smoke_val, buf);
    }
}

void UI_UpdateActuator(uint8_t led, uint8_t buzzer)
{
    /* 更新开关状态（LVGL v9：用 LV_STATE_CHECKED） */
    if (led)
        lv_obj_add_state(sw_led, LV_STATE_CHECKED);
    else
        lv_obj_clear_state(sw_led, LV_STATE_CHECKED);

    if (buzzer)
        lv_obj_add_state(sw_buzzer, LV_STATE_CHECKED);
    else
        lv_obj_clear_state(sw_buzzer, LV_STATE_CHECKED);
}

void UI_UpdateMode(uint8_t mode)
{
    lv_label_set_text(label_mode_val, mode ? "Manual" : "Auto");
    lv_label_set_text_fmt(label_mode_ctrl, "Mode: %s", mode ? "Manual" : "Auto");
}

void UI_UpdateNetwork(uint8_t wifi, uint8_t mqtt)
{
    lv_label_set_text(label_wifi_status, wifi ? "WiFi: OK" : "WiFi: --");
    lv_label_set_text(label_mqtt_status, mqtt ? "MQTT: OK" : "MQTT: --");
    lv_obj_set_style_text_color(label_wifi_status,
                                wifi ? lv_color_hex(0x4CAF50) : lv_color_hex(0x9E9E9E), 0);
    lv_obj_set_style_text_color(label_mqtt_status,
                                mqtt ? lv_color_hex(0x4CAF50) : lv_color_hex(0x9E9E9E), 0);
}
