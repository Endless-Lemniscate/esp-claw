/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Waveshare ESP32-S3-Touch-AMOLED-1.43C board-specific factories.
 */

#include <string.h>
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_sh8601.h"
#include "esp_lcd_touch_cst816s.h"
#include "esp_log.h"

static const char *TAG = "WS_AMOLED_143C";

#define LCD_PHYS_RES              466
#define LCD_ROUND_SAFE_RES        326
#define LCD_ROUND_SAFE_OFFSET     ((LCD_PHYS_RES - LCD_ROUND_SAFE_RES) / 2)
#define LCD_DRIVER_X_GAP          0x08
#define LCD_ROUND_SAFE_X_GAP      (LCD_DRIVER_X_GAP + LCD_ROUND_SAFE_OFFSET)
#define LCD_ROUND_SAFE_Y_GAP      LCD_ROUND_SAFE_OFFSET

static const sh8601_lcd_init_cmd_t lcd_init_cmds[] = {
    {0xFE, (uint8_t []){0x00}, 1, 0},
    {0xC4, (uint8_t []){0x80}, 1, 0},
    {0x3A, (uint8_t []){0x55}, 1, 0},
    {0x35, (uint8_t []){0x00}, 1, 0},
    {0x53, (uint8_t []){0x20}, 1, 0},
    {0x51, (uint8_t []){0xFF}, 1, 0},
    {0x36, (uint8_t []){0xC0}, 1, 0},
    {0x63, (uint8_t []){0xFF}, 1, 0},
    {0x2A, (uint8_t []){0x00, 0x06, 0x01, 0xD7}, 4, 0},
    {0x2B, (uint8_t []){0x00, 0x00, 0x01, 0xD1}, 4, 0},
    {0x11, (uint8_t []){0x00}, 0, 100},
    {0x29, (uint8_t []){0x00}, 0, 0},
};

static const sh8601_vendor_config_t vendor_config = {
    .init_cmds = lcd_init_cmds,
    .init_cmds_size = sizeof(lcd_init_cmds) / sizeof(lcd_init_cmds[0]),
    .flags = {
        .use_qspi_interface = 1,
    },
};

esp_err_t lcd_panel_factory_entry_t(esp_lcd_panel_io_handle_t io,
                                    const esp_lcd_panel_dev_config_t *panel_dev_config,
                                    esp_lcd_panel_handle_t *ret_panel)
{
    esp_lcd_panel_dev_config_t cfg = {0};
    memcpy(&cfg, panel_dev_config, sizeof(esp_lcd_panel_dev_config_t));
    cfg.vendor_config = (void *)&vendor_config;

    esp_err_t ret = esp_lcd_new_panel_sh8601(io, &cfg, ret_panel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_lcd_new_panel_sh8601 failed: %s", esp_err_to_name(ret));
        return ret;
    }

    ret = esp_lcd_panel_set_gap(*ret_panel, LCD_ROUND_SAFE_X_GAP, LCD_ROUND_SAFE_Y_GAP);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "esp_lcd_panel_set_gap failed: %s", esp_err_to_name(ret));
    }
    return ESP_OK;
}

static void cst820_process_coordinates(esp_lcd_touch_handle_t tp,
                                       uint16_t *x,
                                       uint16_t *y,
                                       uint16_t *strength,
                                       uint8_t *point_num,
                                       uint8_t max_point_num)
{
    (void)tp;
    (void)strength;

    uint8_t out = 0;
    for (uint8_t i = 0; i < *point_num && i < max_point_num; i++) {
        int mapped_x = (LCD_PHYS_RES - 1) - (int)x[i] - LCD_ROUND_SAFE_OFFSET;
        int mapped_y = (int)y[i] - LCD_ROUND_SAFE_OFFSET;
        if (mapped_x < 0 || mapped_x >= LCD_ROUND_SAFE_RES ||
                mapped_y < 0 || mapped_y >= LCD_ROUND_SAFE_RES) {
            continue;
        }
        x[out] = (uint16_t)mapped_x;
        y[out] = (uint16_t)mapped_y;
        if (strength != NULL) {
            strength[out] = strength[i];
        }
        out++;
    }
    *point_num = out;
}

esp_err_t lcd_touch_factory_entry_t(esp_lcd_panel_io_handle_t io,
                                    const esp_lcd_touch_config_t *touch_dev_config,
                                    esp_lcd_touch_handle_t *ret_touch)
{
    esp_lcd_touch_config_t touch_cfg = {0};
    memcpy(&touch_cfg, touch_dev_config, sizeof(esp_lcd_touch_config_t));

    touch_cfg.process_coordinates = cst820_process_coordinates;

    esp_err_t ret = esp_lcd_touch_new_i2c_cst816s(io, &touch_cfg, ret_touch);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "esp_lcd_touch_new_i2c_cst816s failed: %s", esp_err_to_name(ret));
        return ret;
    }
    return ESP_OK;
}
