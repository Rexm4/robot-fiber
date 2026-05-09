#include "infrared.h"

#include <driver/gpio.h>
#include <esp_adc/adc_oneshot.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

static const char* TAG = "IR";

static uint16_t s_threshold                   = IR_ANALOG_THRESHOLD_DEFAULT;
static adc_oneshot_unit_handle_t s_adc_handle = NULL;

void infrared_init() {
  gpio_config_t config = {
      .pin_bit_mask = (1ULL << IR_DIGITAL_PIN),
      .mode         = GPIO_MODE_INPUT,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .pull_up_en   = GPIO_PULLUP_DISABLE,
      .intr_type    = GPIO_INTR_DISABLE,
  };
  gpio_config(&config);
  ESP_LOGI(TAG, "Digital mode initialized GPIO%d", IR_DIGITAL_PIN);

  adc_oneshot_unit_init_cfg_t unit_config = {
      .unit_id  = IR_ADC_UNIT,
      .ulp_mode = ADC_ULP_MODE_DISABLE,
  };
  ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_config, &s_adc_handle));

  adc_oneshot_chan_cfg_t chan_config = {
      .atten    = IR_ADC_ATTEN,
      .bitwidth = IR_ADC_WIDTH,
  };
  ESP_ERROR_CHECK(adc_oneshot_config_channel(s_adc_handle, IR_ADC_CHANNEL, &chan_config));

  ESP_LOGI(TAG, "Analog mode initialized ADC%d channel %d", IR_ADC_UNIT + 1, IR_ADC_CHANNEL);
}

void infrared_read(ir_reading_t* out) {
  out->digital_value = 0;
  out->analog_raw    = 0;
  out->gray_value    = 0.0f;
  out->line_detected = false;

  int level          = gpio_get_level(IR_DIGITAL_PIN);  // 0 = white | 1 = black
  out->digital_value = (uint8_t)level;

  uint32_t acc = 0;
  int raw;
  for (int i = 0; i < IR_ADC_OVERSAMPLE; i++) {
    ESP_ERROR_CHECK(adc_oneshot_read(s_adc_handle, IR_ADC_CHANNEL, &raw));
    acc += raw;
  }
  uint16_t avg    = (uint16_t)(acc / IR_ADC_OVERSAMPLE);
  out->analog_raw = avg;

  out->gray_value    = (0.5 * avg) / s_threshold;
  out->line_detected = (avg < IR_ANALOG_THRESHOLD_ON_AIR);
}

void infrared_calibrate_analog(uint16_t samples_per_phase, uint32_t phase_delay_ms) {
  int raw;
  uint32_t acc;
  uint16_t avg;

  // White calibration
  ESP_LOGI(TAG, "CALIBRATION place the sensor on white");
  ESP_LOGI(TAG, "Waiting %lu ms", (unsigned long)phase_delay_ms);
  vTaskDelay(pdMS_TO_TICKS(phase_delay_ms));
  ESP_LOGI(TAG, "CALIBRATING WHITE...");

  acc = 0;
  for (uint16_t i = 0; i < samples_per_phase; i++) {
    ESP_ERROR_CHECK(adc_oneshot_read(s_adc_handle, IR_ADC_CHANNEL, &raw));
    acc += raw;
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  avg = (uint16_t)(acc / samples_per_phase);

  uint16_t avg_white = avg;
  ESP_LOGI(TAG, "White avarage: %d", avg_white);

  // Black calibration
  ESP_LOGI(TAG, "CALIBRATION place the sensor on black");
  ESP_LOGI(TAG, "Waiting %lu ms", (unsigned long)phase_delay_ms);
  vTaskDelay(pdMS_TO_TICKS(phase_delay_ms));
  ESP_LOGI(TAG, "CALIBRATING BLACK...");

  acc = 0;
  for (uint16_t i = 0; i < samples_per_phase; i++) {
    ESP_ERROR_CHECK(adc_oneshot_read(s_adc_handle, IR_ADC_CHANNEL, &raw));
    acc += raw;
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  avg = (uint16_t)(acc / samples_per_phase);

  uint16_t avg_black = avg;
  ESP_LOGI(TAG, "Black avarage: %d", avg_black);

  // Calculate threshold
  s_threshold = (avg_white + avg_black) / 2;
  ESP_LOGI(TAG, "Complete calibration. Threshold: %d", s_threshold);
}

uint16_t infrared_get_threshold(void) {
  return s_threshold;
}