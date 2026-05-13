#include "buttons.h"

#include <driver/gpio.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/queue.h>
#include <freertos/task.h>

#define BTN_GPIO_UP   GPIO_NUM_1
#define BTN_GPIO_DOWN GPIO_NUM_2
#define BTN_GPIO_OK   GPIO_NUM_3

#define BTN_DEBOUNCE_MS 50
#define BTN_QUEUE_LEN   8

static const char* TAG = "buttons";

static QueueHandle_t s_evt_queue = NULL;

typedef struct {
  gpio_num_t gpio;
  ButtonEvent event;
  TickType_t last_tick;
} BtnState;

static BtnState s_btns[3] = {
    {BTN_GPIO_UP, BTN_EVT_UP, 0},
    {BTN_GPIO_DOWN, BTN_EVT_DOWN, 0},
    {BTN_GPIO_OK, BTN_EVT_OK, 0},
};

static void IRAM_ATTR gpio_isr_handler(void* arg) {
  BtnState* btn = (BtnState*)arg;

  TickType_t now = xTaskGetTickCountFromISR();

  if ((now - btn->last_tick) < pdMS_TO_TICKS(BTN_DEBOUNCE_MS))
    return;
  btn->last_tick = now;

  if (gpio_get_level(btn->gpio) != 0)
    return;

  BaseType_t higher_prio_task_woken = pdFALSE;
  xQueueSendFromISR(s_evt_queue, &btn->event, &higher_prio_task_woken);
  if (higher_prio_task_woken)
    portYIELD_FROM_ISR();
}

void buttons_init(void) {
  s_evt_queue = xQueueCreate(BTN_QUEUE_LEN, sizeof(ButtonEvent));
  if (!s_evt_queue) {
    ESP_LOGE(TAG, "Failed creating queue");
    return;
  }

  gpio_install_isr_service(0);

  gpio_config_t config = {
      .intr_type    = GPIO_INTR_NEGEDGE, /* Flanco de bajada (HIGH→LOW) */
      .mode         = GPIO_MODE_INPUT,
      .pull_up_en   = GPIO_PULLUP_ENABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .pin_bit_mask = 0,
  };

  for (int i = 0; i < 3; i++) {
    config.pin_bit_mask = (1ULL << s_btns[i].gpio);
    ESP_ERROR_CHECK(gpio_config(&config));
    ESP_ERROR_CHECK(gpio_isr_handler_add(s_btns[i].gpio, gpio_isr_handler, (void*)&s_btns[i]));
    ESP_LOGI(TAG, "Botón GPIO %d registrado", s_btns[i].gpio);
  }
}

BaseType_t buttons_wait_event(ButtonEvent* out_event, TickType_t timeout_ms) {
  TickType_t ticks = (timeout_ms == portMAX_DELAY) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
  return xQueueReceive(s_evt_queue, out_event, ticks);
}

void buttons_flush(void) {
  ButtonEvent dummy;
  while (xQueueReceive(s_evt_queue, &dummy, 0) == pdTRUE) {}
}