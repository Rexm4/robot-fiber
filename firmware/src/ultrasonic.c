#include "ultrasonic.h"

#include <driver/gpio.h>
#include <esp_log.h>
#include <esp_timer.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include <freertos/task.h>
#include <rom/ets_sys.h>

static const char* TAG = "ULTRASONIC";

static SemaphoreHandle_t s_echo_sem = NULL;

static volatile int64_t s_echo_start_us = 0;
static volatile int64_t s_echo_end_us   = 0;

static void IRAM_ATTR echo_isr_handler(void* arg) {
  if (gpio_get_level(ULTRASONIC_ECHO_PIN)) {
    s_echo_start_us = esp_timer_get_time();
  } else {
    s_echo_end_us = esp_timer_get_time();

    BaseType_t higher_task_woken = pdFALSE;
    xSemaphoreGiveFromISR(s_echo_sem, &higher_task_woken);
    portYIELD_FROM_ISR(higher_task_woken);
  }
}

void ultrasonic_init(void) {
  s_echo_sem = xSemaphoreCreateBinary();

  // TRIG output
  gpio_config_t trig_config = {
      .pin_bit_mask = (1ULL << ULTRASONIC_TRIG_PIN),
      .mode         = GPIO_MODE_OUTPUT,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .pull_up_en   = GPIO_PULLUP_DISABLE,
      .intr_type    = GPIO_INTR_DISABLE,
  };
  gpio_config(&trig_config);
  gpio_set_level(ULTRASONIC_TRIG_PIN, 0);

  // ECHO input
  gpio_config_t echo_config = {
      .pin_bit_mask = (1ULL << ULTRASONIC_ECHO_PIN),
      .mode         = GPIO_MODE_INPUT,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .pull_up_en   = GPIO_PULLUP_DISABLE,
      .intr_type    = GPIO_INTR_ANYEDGE,
  };
  gpio_config(&echo_config);

  gpio_install_isr_service(0);
  gpio_isr_handler_add(ULTRASONIC_ECHO_PIN, echo_isr_handler, NULL);

  ESP_LOGI(TAG, "Initialized TRIG: GPIO%d, ECHO: GPIO%d", ULTRASONIC_TRIG_PIN, ULTRASONIC_ECHO_PIN);
}

void ultrasonic_measure(ultrasonic_reading_t* out) {
  out->distance_cm = 0.0f;
  out->valid       = false;

  xSemaphoreTake(s_echo_sem, 0);

  gpio_set_level(ULTRASONIC_TRIG_PIN, 0);
  ets_delay_us(4);
  gpio_set_level(ULTRASONIC_TRIG_PIN, 1);
  ets_delay_us(10);
  gpio_set_level(ULTRASONIC_TRIG_PIN, 0);

  if (xSemaphoreTake(s_echo_sem, pdMS_TO_TICKS(ULTRASONIC_TIMEOUT_MS)) == pdFALSE) {
    //ESP_LOGW(TAG, "Timeout waiting ECHO pulse");
    return;
  }

  int64_t duration_us = s_echo_end_us - s_echo_start_us;

  if (duration_us <= 0) {
    ESP_LOGW(TAG, "Invalid pulse duration: %lld µs", duration_us);

    return;
  }

  float distance_cm = (float)duration_us / ULTRASONIC_US_PER_CM;

  if (distance_cm > ULTRASONIC_MAX_DIST) {
    ESP_LOGW(TAG, "Out of range: %.1f cm", distance_cm);
    return;
  }

  out->distance_cm = distance_cm;
  out->valid       = true;

  ESP_LOGD(TAG, "Distance: %.1f cm (%lld µs) ", distance_cm, duration_us);
}

float ultrasonic_get_cm(void) {
  ultrasonic_reading_t r;
  ultrasonic_measure(&r);
  return r.valid ? r.distance_cm : -1.0f;
}