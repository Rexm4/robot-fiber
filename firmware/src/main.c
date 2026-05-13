#include <driver/i2c_master.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <stdio.h>

#include "buttons.h"
#include "comms.h"
#include "infrared.h"
#include "menu.h"
#include "motors.h"
#include "oled.h"
#include "ultrasonic.h"

#include "mode_linear_follower.h"
#include "mode_obstacle_avoider.h"
#include "mode_wifi.h"

//static const char* TAG = "MAIN";

#define I2C_SCL_PIN  GPIO_NUM_9
#define I2C_SDA_PIN  GPIO_NUM_8
#define I2C_PORT_NUM I2C_NUM_0

static esp_err_t i2c_bus_create(i2c_master_bus_handle_t* bus_handle) {
  i2c_master_bus_config_t busConfig = {
      .i2c_port                     = I2C_PORT_NUM,
      .scl_io_num                   = I2C_SCL_PIN,
      .sda_io_num                   = I2C_SDA_PIN,
      .clk_source                   = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt            = 0,
      .flags.enable_internal_pullup = true,
  };
  return i2c_new_master_bus(&busConfig, bus_handle);
}

void app_main() {
  vTaskDelay(pdMS_TO_TICKS(3000));
  /*
  motors_init();
  */

  buttons_init();

  infrared_init();
  //infrared_calibrate_analog(20, 5000);
  ir_reading_t ir;

  ultrasonic_init();

  i2c_master_bus_handle_t bus_handle;
  i2c_bus_create(&bus_handle);

  i2c_master_dev_handle_t oled_handle;
  oled_init(bus_handle, &oled_handle);
  /*
   for (;;) {

    infrared_read(&ir);

    printf("[IR] analog_raw: %4d | gray_value: %.2f | line_detected: %d || [ULTRASONIC] CM: %.2f\n",
           ir.analog_raw, ir.gray_value, ir.line_detected, ultrasonic_get_cm());
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
 */

  RobotMode mode = menu_get_mode();

  switch (mode) {
    case MODE_LINEAR_FOLLOWER:
      //linear_follower_run();
      printf("Running linear follower mode\n");
      break;
    case MODE_OBSTACLE_AVOIDER:
      //obstacle_avoider_run();
      printf("Running obstacle avoider mode\n");
      break;
    case MODE_WIFI:
      //wifi_run();
      printf("Running wifi mode\n");
      break;
    default:
      printf("ERROR Unknown mode\n");
  }
}