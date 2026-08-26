/**
 * @file waveshare_lcd.c
 * @brief HAL implementation for Waveshare ESP32-S3-Touch-LCD-4.3B
 *
 * Initialises:
 *   - ST7262 RGB panel via esp_lcd panel API (parallel RGB)
 *   - GT911 touch controller via I2C in polling mode
 *   - LEDC PWM backlight
 *   - LVGL display and input device drivers
 *
 * LVGL flush callback uses DMA-backed frame buffer in PSRAM.
 */

#include "waveshare_lcd.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/ledc.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_rgb.h"
#include "esp_log.h"
#include "lvgl.h"

static const char *TAG = "waveshare_lcd";

/* ── GT911 touch registers ─────────────────────────────────────────── */
#define GT911_ADDR          0x5D
#define GT911_STATUS_REG    0x814E
#define GT911_POINT1_REG    0x814F
#define GT911_TOUCH_POINTS  5

/* ── Frame buffers (double-buffer in PSRAM) ─────────────────────────── */
static lv_color_t *s_fb[2];
#define FB_SIZE  (LCD_H_RES * LCD_V_RES * sizeof(lv_color_t))

/* ── LCD panel handle ───────────────────────────────────────────────── */
static esp_lcd_panel_handle_t s_panel = NULL;

/* ── Flush callback ─────────────────────────────────────────────────── */
static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area,
                           lv_color_t *color_map)
{
    esp_lcd_panel_draw_bitmap(s_panel,
                              area->x1, area->y1,
                              area->x2 + 1, area->y2 + 1,
                              color_map);
    lv_disp_flush_ready(drv);
}

/* ── GT911 helpers ──────────────────────────────────────────────────── */
static esp_err_t gt911_read(uint16_t reg, uint8_t *buf, size_t len)
{
    uint8_t addr[2] = { reg >> 8, reg & 0xFF };
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (GT911_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, addr, 2, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (GT911_ADDR << 1) | I2C_MASTER_READ, true);
    i2c_master_read(cmd, buf, len, I2C_MASTER_LAST_NACK);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(TOUCH_I2C_PORT, cmd, pdMS_TO_TICKS(50));
    i2c_cmd_link_delete(cmd);
    return ret;
}

static esp_err_t gt911_write_byte(uint16_t reg, uint8_t val)
{
    uint8_t buf[3] = { reg >> 8, reg & 0xFF, val };
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (GT911_ADDR << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write(cmd, buf, 3, true);
    i2c_master_stop(cmd);
    esp_err_t ret = i2c_master_cmd_begin(TOUCH_I2C_PORT, cmd, pdMS_TO_TICKS(50));
    i2c_cmd_link_delete(cmd);
    return ret;
}

/* ── Touch read (LVGL indev driver callback) ────────────────────────── */
void waveshare_lcd_touch_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    (void)drv;

    uint8_t status = 0;
    gt911_read(GT911_STATUS_REG, &status, 1);

    uint8_t touch_count = status & 0x0F;
    bool buffer_ready   = (status & 0x80) != 0;

    if (!buffer_ready || touch_count == 0) {
        data->state = LV_INDEV_STATE_REL;
        /* clear status register */
        gt911_write_byte(GT911_STATUS_REG, 0x00);
        return;
    }

    /* Read first touch point (8 bytes per point) */
    uint8_t point[8];
    gt911_read(GT911_POINT1_REG, point, 8);

    /* GT911 gives X,Y as 16-bit LE */
    uint16_t x = point[1] | (point[2] << 8);
    uint16_t y = point[3] | (point[4] << 8);

    /* Clamp to screen */
    if (x >= LCD_H_RES) x = LCD_H_RES - 1;
    if (y >= LCD_V_RES) y = LCD_V_RES - 1;

    data->point.x = (lv_coord_t)x;
    data->point.y = (lv_coord_t)y;
    data->state   = LV_INDEV_STATE_PR;

    gt911_write_byte(GT911_STATUS_REG, 0x00);
}

/* ── Backlight ──────────────────────────────────────────────────────── */
void waveshare_lcd_set_backlight(uint8_t percent)
{
    if (percent > 100) percent = 100;
    uint32_t duty = (percent * ((1 << 10) - 1)) / 100;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LCD_BL_LEDC_CH, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LCD_BL_LEDC_CH);
}

/* ── Main init ──────────────────────────────────────────────────────── */
void waveshare_lcd_init(void)
{
    ESP_LOGI(TAG, "Initialising Waveshare 4.3\" LCD");

    /* ── Allocate frame buffers in PSRAM ── */
    s_fb[0] = heap_caps_malloc(FB_SIZE, MALLOC_CAP_SPIRAM);
    s_fb[1] = heap_caps_malloc(FB_SIZE, MALLOC_CAP_SPIRAM);
    assert(s_fb[0] && s_fb[1]);
    memset(s_fb[0], 0, FB_SIZE);
    memset(s_fb[1], 0, FB_SIZE);

    /* ── RGB panel ── */
    esp_lcd_rgb_panel_config_t cfg = {
        .clk_src            = LCD_CLK_SRC_DEFAULT,
        .timings = {
            .pclk_hz        = 16 * 1000 * 1000,
            .h_res          = LCD_H_RES,
            .v_res          = LCD_V_RES,
            .hsync_back_porch  = 40,
            .hsync_front_porch = 20,
            .hsync_pulse_width = 1,
            .vsync_back_porch  = 8,
            .vsync_front_porch = 4,
            .vsync_pulse_width = 1,
            .flags.pclk_active_neg = true,
        },
        .data_width  = 16,
        .num_fbs     = 2,
        .psram_trans_align = 64,
        .hsync_gpio_num  = LCD_HSYNC_GPIO,
        .vsync_gpio_num  = LCD_VSYNC_GPIO,
        .de_gpio_num     = LCD_DE_GPIO,
        .pclk_gpio_num   = LCD_PCLK_GPIO,
        .data_gpio_nums  = {
            LCD_DATA0_GPIO,  LCD_DATA1_GPIO,  LCD_DATA2_GPIO,
            LCD_DATA3_GPIO,  LCD_DATA4_GPIO,  LCD_DATA5_GPIO,
            LCD_DATA6_GPIO,  LCD_DATA7_GPIO,  LCD_DATA8_GPIO,
            LCD_DATA9_GPIO,  LCD_DATA10_GPIO, LCD_DATA11_GPIO,
            LCD_DATA12_GPIO, LCD_DATA13_GPIO, LCD_DATA14_GPIO,
            LCD_DATA15_GPIO,
        },
        .disp_gpio_num = LCD_DISP_EN_GPIO,
        .flags.fb_in_psram = true,
    };
    ESP_ERROR_CHECK(esp_lcd_new_rgb_panel(&cfg, &s_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(s_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_init(s_panel));
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(s_panel, true));

    /* ── Backlight via LEDC PWM ── */
    ledc_timer_config_t bl_timer = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .timer_num       = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz         = 5000,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&bl_timer);

    ledc_channel_config_t bl_ch = {
        .gpio_num   = LCD_BL_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = LCD_BL_LEDC_CH,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = 0,
        .hpoint     = 0,
    };
    ledc_channel_config(&bl_ch);
    waveshare_lcd_set_backlight(80);

    /* ── I2C for GT911 touch ── */
    i2c_config_t i2c_cfg = {
        .mode             = I2C_MODE_MASTER,
        .sda_io_num       = TOUCH_SDA_GPIO,
        .scl_io_num       = TOUCH_SCL_GPIO,
        .sda_pullup_en    = GPIO_PULLUP_ENABLE,
        .scl_pullup_en    = GPIO_PULLUP_ENABLE,
        .master.clk_speed = TOUCH_I2C_FREQ,
    };
    ESP_ERROR_CHECK(i2c_param_config(TOUCH_I2C_PORT, &i2c_cfg));
    ESP_ERROR_CHECK(i2c_driver_install(TOUCH_I2C_PORT, I2C_MODE_MASTER, 0, 0, 0));

    if (TOUCH_RST_GPIO >= 0) {
        gpio_reset_pin(TOUCH_RST_GPIO);
        gpio_set_direction(TOUCH_RST_GPIO, GPIO_MODE_OUTPUT);
        gpio_set_level(TOUCH_RST_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(10));
        gpio_set_level(TOUCH_RST_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    /* ── LVGL init ── */
    lv_init();

    static lv_disp_draw_buf_t draw_buf;
    lv_disp_draw_buf_init(&draw_buf, s_fb[0], s_fb[1],
                          LCD_H_RES * LCD_V_RES);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res    = LCD_H_RES;
    disp_drv.ver_res    = LCD_V_RES;
    disp_drv.flush_cb   = lvgl_flush_cb;
    disp_drv.draw_buf   = &draw_buf;
    disp_drv.full_refresh = 1;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type     = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb  = waveshare_lcd_touch_read;
    lv_indev_drv_register(&indev_drv);

    ESP_LOGI(TAG, "LCD ready  %dx%d", LCD_H_RES, LCD_V_RES);
}
