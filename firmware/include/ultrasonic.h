#ifndef ULTRASONIC_H__
#define ULTRASONIC_H__

#include <stdbool.h>
#include <stdint.h>

#define ULTRASONIC_TRIG_PIN GPIO_NUM_12
#define ULTRASONIC_ECHO_PIN GPIO_NUM_13

#define ULTRASONIC_MAX_DIST   400.0f
#define ULTRASONIC_TIMEOUT_MS 30
#define ULTRASONIC_US_PER_CM  58.0f

typedef struct {
  float distance_cm;
  bool valid;
} ultrasonic_reading_t;

void ultrasonic_init(void);

void ultrasonic_measure(ultrasonic_reading_t* out);

float ultrasonic_get_cm(void);

#endif  // ULTRASONIC_H__