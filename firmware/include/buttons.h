#ifndef BUTTONS_H__
#define BUTTONS_H__

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

typedef enum {
  BTN_EVT_UP   = 0,
  BTN_EVT_DOWN = 1,
  BTN_EVT_OK   = 2,
} ButtonEvent;

void buttons_init(void);

BaseType_t buttons_wait_event(ButtonEvent* out_event, TickType_t timeout_ms);

void buttons_flush(void);

#endif  // BUTTONS_H__
