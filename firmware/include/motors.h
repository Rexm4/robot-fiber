#ifndef MOTORS_H__
#define MOTORS_H__

#include <driver/gpio.h>
#include <driver/ledc.h>
#include <esp_err.h>
#include <stdint.h>

// FRONT RIGHT MOTOR
#define MOTOR_FR_IN1 GPIO_NUM_42
#define MOTOR_FR_IN2 GPIO_NUM_41
#define MOTOR_FR_PWM GPIO_NUM_40
#define MOTOR_FR_CH  LEDC_CHANNEL_0

// FRONT LEFT MOTOR
#define MOTOR_FL_IN1 GPIO_NUM_37
#define MOTOR_FL_IN2 GPIO_NUM_36
#define MOTOR_FL_PWM GPIO_NUM_35
#define MOTOR_FL_CH  LEDC_CHANNEL_1

// REAR RIGTH MOTOR
#define MOTOR_RR_IN1 GPIO_NUM_45
#define MOTOR_RR_IN2 GPIO_NUM_48
#define MOTOR_RR_PWM GPIO_NUM_47
#define MOTOR_RR_CH  LEDC_CHANNEL_2

// REAR LEFT MOTOR
#define MOTOR_RL_IN1 GPIO_NUM_21
#define MOTOR_RL_IN2 GPIO_NUM_20
#define MOTOR_RL_PWM GPIO_NUM_19
#define MOTOR_RL_CH  LEDC_CHANNEL_3

#define PWM_HZ 500

typedef enum {
  MOTOR_FR = 0,
  MOTOR_FL,
  MOTOR_RR,
  MOTOR_RL,
  MOTOR_COUNT,
} motor_id_t;

typedef enum {
  MOTOR_DIR_FORWARD  = 0,
  MOTOR_DIR_BACKWARD = 1,
} motor_dir_t;

typedef struct {
  gpio_num_t in1_pin;
  gpio_num_t in2_pin;
  gpio_num_t pwm_pin;
  ledc_channel_t ledc_channel;
} motor_config_t;

esp_err_t motors_init(void);

void motor_set(motor_id_t id, motor_dir_t dir, uint8_t speed);

void motor_stop(motor_id_t id);
void motro_break(motor_id_t id);

void motor_forward(uint8_t speed);
void motor_backwards(uint8_t speed);
void motor_turn_left(uint8_t speed);
void motor_turn_right(uint8_t speed);

void stop_all(void);

#endif  // MOTORS_H__