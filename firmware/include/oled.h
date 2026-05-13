#ifndef OLED_H__
#define OLED_H__

#include <driver/i2c_master.h>
#include <esp_err.h>
#include <stdint.h>

#define OLED_I2C_ADDR 0x3C
#define OLED_SCL_HZ   100000

#define OLED_WIDTH  128
#define OLED_HEIGHT 64
#define OLED_PAGES  (OLED_HEIGHT / 8)  // 8 pages of 8 rows

esp_err_t oled_init(i2c_master_bus_handle_t bus_handle, i2c_master_dev_handle_t *dev_handle);

void oled_clear();
esp_err_t oled_flush(void);

void oled_draw_bitmap(const uint8_t* bitmap);
void oled_draw_pixel(int x, int y, uint8_t color);
void oled_draw_char(int x, int y, char c, uint8_t scale, uint8_t color);
void oled_draw_string(int x, int y, const char* str, uint8_t scale, uint8_t color);
void oled_draw_hline(int x, int y, int w, uint8_t color);
void oled_draw_rect(int x0, int y0, int x1, int y1, uint8_t color);
void oled_fill_rect(int x0, int y0, int x1, int y1, uint8_t color);

#endif  // OLED_H__