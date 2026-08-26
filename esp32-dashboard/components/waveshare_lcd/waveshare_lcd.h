/**
 * @file waveshare_lcd.h
 * @brief HAL for Waveshare ESP32-S3-Touch-LCD-4.3B
 *
 * Display : 800x480, RGB565, ST7262 (parallel RGB interface)
 * Touch   : GT911, I2C
 * Backlight: GPIO controlled via LEDC PWM
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "lvgl.h"

/* ── Pin assignments (Waveshare ESP32-S3-Touch-LCD-4.3B) ─────────────── */

/* RGB parallel display */
#define LCD_HSYNC_GPIO   39
#define LCD_VSYNC_GPIO   41
#define LCD_DE_GPIO      40
#define LCD_PCLK_GPIO    42
#define LCD_DATA0_GPIO   8   /* B0 */
#define LCD_DATA1_GPIO   3   /* B1 */
#define LCD_DATA2_GPIO   46  /* B2 */
#define LCD_DATA3_GPIO   9   /* B3 */
#define LCD_DATA4_GPIO   1   /* B4 */
#define LCD_DATA5_GPIO   5   /* G0 */
#define LCD_DATA6_GPIO   6   /* G1 */
#define LCD_DATA7_GPIO   7   /* G2 */
#define LCD_DATA8_GPIO   15  /* G3 */
#define LCD_DATA9_GPIO   16  /* G4 */
#define LCD_DATA10_GPIO  4   /* G5 */
#define LCD_DATA11_GPIO  45  /* R0 */
#define LCD_DATA12_GPIO  48  /* R1 */
#define LCD_DATA13_GPIO  47  /* R2 */
#define LCD_DATA14_GPIO  21  /* R3 */
#define LCD_DATA15_GPIO  14  /* R4 */
#define LCD_DISP_EN_GPIO -1  /* not used on this board */

/* Backlight */
#define LCD_BL_GPIO      2
#define LCD_BL_LEDC_CH   0

/* Touch GT911 (I2C) */
#define TOUCH_SCL_GPIO   20
#define TOUCH_SDA_GPIO   19
#define TOUCH_RST_GPIO   38
#define TOUCH_INT_GPIO   -1  /* polling mode */
#define TOUCH_I2C_PORT   I2C_NUM_0
#define TOUCH_I2C_FREQ   400000

/* Resolution */
#define LCD_H_RES        800
#define LCD_V_RES        480

/**
 * Initialise the display, touch controller and LVGL driver.
 * Must be called once from the UI task before any lv_* calls.
 */
void waveshare_lcd_init(void);

/**
 * Set backlight brightness 0–100 %.
 */
void waveshare_lcd_set_backlight(uint8_t percent);

/**
 * Call periodically (every ~5 ms) to feed touch events into LVGL.
 * Suitable to call from inside an lv_timer or from the main LVGL tick.
 */
void waveshare_lcd_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data);
