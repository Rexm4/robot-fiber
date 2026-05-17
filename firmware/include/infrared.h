#ifndef INFRARED_H__
#define INFRARED_H__

#include <esp_adc/adc_oneshot.h>
#include <stdbool.h>
#include <stdint.h>

#define IR_DIGITAL_PIN GPIO_NUM_11
#define IR_ADC_CHANNEL ADC_CHANNEL_9  // GPIO10
#define IR_ADC_UNIT    ADC_UNIT_1
#define IR_ADC_WIDTH   ADC_BITWIDTH_12  // 0 - 4095
#define IR_ADC_ATTEN   ADC_ATTEN_DB_12  // TODO verify

#define IR_LINE_LEVEL 1

#define IR_ANALOG_THRESHOLD_DEFAULT 2048
#define IR_ANALOG_THRESHOLD_ON_AIR  3900
#define IR_ADC_OVERSAMPLE           8

typedef struct {
  uint8_t digital_value;
  uint16_t analog_raw;
  float gray_value;
  bool line_detected;
} ir_reading_t;

void infrared_init(void);

void infrared_read(ir_reading_t* out);

void infrared_calibrate_analog(uint16_t samples_per_phase, uint32_t phase_delay_ms);

uint16_t infrared_get_threshold(void);

#endif  // INFRARED_H__