#include "motors.h"

#include <esp_log.h>

#define LEDC_DUTY_MAX ((1 << 10) - 1)  // 1023

static bool g_initialized = false;

static const char* TAG = "MOTORS";

static motor_config_t s_motors[MOTOR_COUNT] = {
    {MOTOR_FR_IN1, MOTOR_FR_IN2, MOTOR_FR_PWM, MOTOR_FR_CH},
    {MOTOR_FL_IN1, MOTOR_FL_IN2, MOTOR_FL_PWM, MOTOR_FL_CH},
    {MOTOR_RR_IN1, MOTOR_RR_IN2, MOTOR_RR_PWM, MOTOR_RR_CH},
    {MOTOR_RL_IN1, MOTOR_RL_IN2, MOTOR_RL_PWM, MOTOR_RL_CH},
};

static inline uint32_t percent_to_duty(uint8_t pct) {
  if (pct > 100)
    pct = 100;
  return ((uint32_t)pct * LEDC_DUTY_MAX) / 100;
}

esp_err_t motors_init(void) {

  ledc_timer_config_t timer_config = {
      .speed_mode      = LEDC_LOW_SPEED_MODE,
      .duty_resolution = LEDC_TIMER_10_BIT,
      .timer_num       = LEDC_TIMER_0,
      .freq_hz         = 200,
      .clk_cfg         = LEDC_AUTO_CLK,
      .deconfigure     = 0,
  };
  esp_err_t ret = ledc_timer_config(&timer_config);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to configure: %s", esp_err_to_name(ret));
    return ret;
  }
  ESP_LOGI(TAG, "Timer initialized successfully");

  ledc_channel_config_t ch_config = {
      .speed_mode          = LEDC_LOW_SPEED_MODE,
      .intr_type           = LEDC_INTR_DISABLE,
      .timer_sel           = LEDC_TIMER_0,
      .duty                = 0,
      .hpoint              = 0,
      .flags.output_invert = 0,
  };

  for (int i = 0; i < 4; i++) {
    ch_config.channel  = s_motors[i].ledc_channel;
    ch_config.gpio_num = s_motors[i].pwm_pin;

    ret = ledc_channel_config(&ch_config);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to initialize channel: %s", esp_err_to_name(ret));
      return ret;
    }
    ESP_LOGI(TAG, "PWM initilized GPIO%d channel %d", s_motors[i].pwm_pin,
             s_motors[i].ledc_channel);
  }

  gpio_config_t config = {
      .mode         = GPIO_MODE_OUTPUT,
      .intr_type    = GPIO_INTR_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_DISABLE,
      .pull_up_en   = GPIO_PULLUP_DISABLE,
  };

  for (int i = 0; i < 4; i++) {
    config.pin_bit_mask = (1ULL << s_motors[i].in1_pin) | (1ULL << s_motors[i].in2_pin);

    ret = gpio_config(&config);
    if (ret != ESP_OK) {
      ESP_LOGE(TAG, "Failed to add GPIO%d | GPIO%d: %s", s_motors[i].in1_pin, s_motors[i].in2_pin,
               esp_err_to_name(ret));
      return ret;
    }
    ESP_LOGI(TAG, "Initialized GPIO%d & GPIO%d", s_motors[i].in1_pin, s_motors[i].in2_pin);
    gpio_set_level(s_motors[i].in1_pin, 0);
    gpio_set_level(s_motors[i].in2_pin, 1);
  }

  g_initialized = true;
  return ESP_OK;
}

void motor_set(motor_id_t id, motor_dir_t dir, uint8_t speed) {
  if (!g_initialized || id >= MOTOR_COUNT)
    return;

  uint32_t duty = percent_to_duty(speed);

  if (dir == MOTOR_DIR_FORWARD) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, s_motors[id].ledc_channel, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, s_motors[id].ledc_channel);
    gpio_set_level(s_motors[id].in1_pin, 1);
    gpio_set_level(s_motors[id].in2_pin, 0);

  } else {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, s_motors[id].ledc_channel, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, s_motors[id].ledc_channel);
    gpio_set_level(s_motors[id].in1_pin, 0);
    gpio_set_level(s_motors[id].in2_pin, 1);
  }
}

void motor_stop(motor_id_t id) {
  if (!g_initialized || id >= MOTOR_COUNT)
    return;

  ledc_set_duty(LEDC_LOW_SPEED_MODE, s_motors[id].ledc_channel, 0);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, s_motors[id].ledc_channel);
  gpio_set_level(s_motors[id].in1_pin, 0);
  gpio_set_level(s_motors[id].in2_pin, 0);
}

void motor_brake(motor_id_t id) {
  if (!g_initialized || id >= MOTOR_COUNT)
    return;

  ledc_set_duty(LEDC_LOW_SPEED_MODE, s_motors[id].ledc_channel, LEDC_DUTY_MAX);
  ledc_update_duty(LEDC_LOW_SPEED_MODE, s_motors[id].ledc_channel);
  gpio_set_level(s_motors[id].in1_pin, 1);
  gpio_set_level(s_motors[id].in2_pin, 1);
}

void motors_forward(uint8_t speed) {
  for (int i = 0; i < MOTOR_COUNT; i++) {
    motor_set((motor_id_t)i, MOTOR_DIR_FORWARD, speed);
  }
}

void motors_backwards(uint8_t speed) {
  for (int i = 0; i < MOTOR_COUNT; i++) {
    motor_set((motor_id_t)i, MOTOR_DIR_BACKWARD, speed);
  }
}

void motors_turn_left(uint8_t speed) {
  motor_set(MOTOR_FR, MOTOR_DIR_FORWARD, speed);
  motor_set(MOTOR_RR, MOTOR_DIR_FORWARD, speed);
  motor_set(MOTOR_FL, MOTOR_DIR_BACKWARD, speed);
  motor_set(MOTOR_RL, MOTOR_DIR_BACKWARD, speed);
}
void motors_turn_right(uint8_t speed) {
  motor_set(MOTOR_FR, MOTOR_DIR_BACKWARD, speed);
  motor_set(MOTOR_RR, MOTOR_DIR_BACKWARD, speed);
  motor_set(MOTOR_FL, MOTOR_DIR_FORWARD, speed);
  motor_set(MOTOR_RL, MOTOR_DIR_FORWARD, speed);
}

void stop_all(void) {}