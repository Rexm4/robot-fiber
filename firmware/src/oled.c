#include "oled.h"
#include "font5x7.h"

#include <esp_log.h>
#include <string.h>

static const char* TAG = "OLED";

static uint8_t s_buffer[OLED_PAGES][OLED_WIDTH];

static i2c_master_dev_handle_t s_dev_handle;

static const uint8_t INIT_SEQ[] = {
    0x00,       /* byte de control: Co=0, D/C#=0  → todos son comandos */
    0xAE,       /* Display OFF                                         */
    0xD5, 0x80, /* Clock divide ratio / oscillator frequency           */
    0xA8, 0x3F, /* Multiplex ratio: 64 líneas (63 = 0x3F)              */
    0xD3, 0x00, /* Display offset: 0                                   */
    0x40,       /* Start line: 0                                       */
    0x8D, 0x14, /* Charge pump: habilitar                              */
    0x20, 0x00, /* Memory addressing mode: Horizontal                  */
    0xA1,       /* Segment re-map: columna 127 → SEG0                  */
    0xC8,       /* COM output scan direction: remapped                 */
    0xDA, 0x12, /* COM pins hardware config                            */
    0x81, 0xCF, /* Contrast: 0xCF                                      */
    0xD9, 0xF1, /* Pre-charge period                                   */
    0xDB, 0x40, /* VCOMH deselect level                                */
    0xA4,       /* Entire display ON: sigue el contenido de RAM        */
    0xA6,       /* Normal display (no invertido)                       */
    0xAF,       /* Display ON                                          */
};

static const uint8_t SET_FULL_PAGE[] = {
    0x00,             /* byte de control: todos comandos */
    0x21, 0x00, 0x7F, /* Column address: 0..127          */
    0x22, 0x00, 0x07, /* Page address:   0..7            */
};

esp_err_t oled_init(i2c_master_bus_handle_t bus_handle, i2c_master_dev_handle_t* dev_handle) {

  i2c_device_config_t config = {
      .dev_addr_length = I2C_ADDR_BIT_7,
      .device_address  = OLED_I2C_ADDR,
      .scl_speed_hz    = OLED_SCL_HZ,
      .scl_wait_us     = 0,
  };
  esp_err_t ret = i2c_master_bus_add_device(bus_handle, &config, dev_handle);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to add device to I2C bus: %s", esp_err_to_name(ret));
    return ret;
  }

  ret = i2c_master_transmit(*dev_handle, INIT_SEQ, sizeof(INIT_SEQ), -1);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize OLED: %s", esp_err_to_name(ret));
    return ret;
  }

  oled_clear();
  s_dev_handle = *dev_handle;
  ESP_LOGI(TAG, "Successfully initialized");
  return ESP_OK;
}

void oled_clear() {
  memset(s_buffer, 0x00, sizeof(s_buffer));
}

esp_err_t oled_flush(void) {
  esp_err_t ret = i2c_master_transmit(s_dev_handle, SET_FULL_PAGE, sizeof(SET_FULL_PAGE), -1);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set page: %s", esp_err_to_name(ret));
    return ret;
  }

  static uint8_t tx_buffer[1 + OLED_PAGES * OLED_WIDTH];
  tx_buffer[0] = 0x40;
  for (int p = 0; p < OLED_PAGES; p++) {
    memcpy(&tx_buffer[1 + p * OLED_WIDTH], s_buffer[p], OLED_WIDTH);
  }

  ret = i2c_master_transmit(s_dev_handle, tx_buffer, sizeof(tx_buffer), -1);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to transmit buffer: %s", esp_err_to_name(ret));
    return ret;
  }
  return ESP_OK;
}

void oled_draw_pixel(int x, int y, uint8_t color) {
  if (x < 0 || x >= OLED_WIDTH || y < 0 || y >= OLED_HEIGHT)
    return;
  int page = y / 8;
  int bit  = y % 8;
  if (color)
    s_buffer[page][x] |= (1 << bit);
  else
    s_buffer[page][x] &= ~(1 << bit);
}

void oled_draw_char(int x, int y, char c, uint8_t scale, uint8_t color) {
  if (c < 32 || c > 126)
    c = '?';
  const uint8_t* glyph = FONT5x7[c - 32];
  for (int col = 0; col < 5; col++) {
    uint8_t line = glyph[col];
    for (int row = 0; row < 7; row++) {
      if (line & (1 << row)) {
        for (int sy = 0; sy < scale; sy++)
          for (int sx = 0; sx < scale; sx++)
            oled_draw_pixel(x + col * scale + sx, y + row * scale + sy, color);
      }
    }
  }
}

void oled_draw_string(int x, int y, const char* str, uint8_t scale, uint8_t color) {
  int cx     = x;
  int char_w = (5 + 1) * scale; /* 5 cols + 1 de espaciado              */
  int char_h = (7 + 1) * scale;

  while (*str) {
    if (cx + char_w > OLED_WIDTH) {
      cx = x;
      y += char_h;
    }
    if (y + char_h > OLED_HEIGHT)
      break;
    oled_draw_char(cx, y, *str, scale, color);
    cx += char_w;
    str++;
  }
}

void oled_draw_hline(int x, int y, int w, uint8_t color) {
  for (int i = 0; i < w; i++)
    oled_draw_pixel(x + i, y, color);
}

void oled_draw_rect(int x0, int y0, int x1, int y1, uint8_t color) {
  oled_draw_hline(x0, y0, x1 - x0 + 1, color);
  oled_draw_hline(x0, y1, x1 - x0 + 1, color);
  for (int y = y0; y <= y1; y++) {
    oled_draw_pixel(x0, y, color);
    oled_draw_pixel(x1, y, color);
  }
}

void oled_fill_rect(int x0, int y0, int x1, int y1, uint8_t color) {
  for (int y = y0; y <= y1; y++)
    oled_draw_hline(x0, y, x1 - x0 + 1, color);
}
