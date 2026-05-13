#include "menu.h"

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "buttons.h"
#include "oled.h"

#define MENU_ITEMS   3
#define MENU_ITEM_H  14
#define MENU_START_Y 22
#define SPLASH_MS    2500

static const char* MENU_LABELS[MENU_ITEMS] = {
    "1. Linea",
    "2. Obstaculos",
    "3. WiFi",
};

static void draw_splash(void) {
  oled_clear();
  oled_draw_rect(0, 0, 127, 63, 1);
  oled_draw_string(36, 2, "FIBER", 2, 1);
  oled_draw_string(36, 18, "ROBOT", 2, 1);
  oled_draw_hline(10, 34, 108, 1);
  oled_draw_string(20, 38, "Selecciona modo", 1, 1);
  oled_draw_string(45, 50, "[ OK ]", 1, 1);
  oled_flush();
}

static void draw_menu(int selected) {
  oled_clear();

  oled_draw_string(2, 1, "Modo:", 1, 1);
  oled_draw_hline(0, 14, 128, 1);

  for (int i = 0; i < MENU_ITEMS; i++) {
    int y = MENU_START_Y + i * MENU_ITEM_H;

    if (i == selected) {
      oled_fill_rect(0, y - 2, 127, y + MENU_ITEM_H - 6, 1);
      oled_draw_string(8, y, MENU_LABELS[i], 1, 0);

      oled_draw_string(1, y, ">", 1, 0);
    } else {
      oled_draw_string(8, y, MENU_LABELS[i], 1, 1);
    }
  }
  oled_flush();
}

RobotMode menu_get_mode(void) {
  draw_splash();
  buttons_flush();
  ButtonEvent ev;
  buttons_wait_event(&ev, SPLASH_MS);

  int selected = 0;
  buttons_flush();
  draw_menu(selected);

  while (1) {
    if (buttons_wait_event(&ev, portMAX_DELAY) != pdTRUE)
      continue;

    switch (ev) {
      case BTN_EVT_UP:
        selected = (selected - 1 + MENU_ITEMS) % MENU_ITEMS;
        draw_menu(selected);
        break;

      case BTN_EVT_DOWN:
        selected = (selected + 1) % MENU_ITEMS;
        draw_menu(selected);
        break;

      case BTN_EVT_OK:
        /* Feedback visual: invertir pantalla brevemente */
        oled_fill_rect(0, 0, 127, 63, 1);
        oled_flush();
        vTaskDelay(pdMS_TO_TICKS(120));
        oled_clear();
        oled_flush();
        vTaskDelay(pdMS_TO_TICKS(80));
        draw_menu(selected);

        /* Devolver el modo (índice base-0 → enum base-1) */
        return (RobotMode)(selected + 1);
    }
  }
}