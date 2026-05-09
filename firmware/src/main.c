#include <freertos/FreeRTOS.h>
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

void app_main() {
  vTaskDelay(pdMS_TO_TICKS(5000));
  /*
  motors_init();

  oled_init();

  buttons_init();
*/

  infrared_init();
  //infrared_calibrate_analog(20, 5000);
  ir_reading_t ir;

  ultrasonic_init();

  for (;;) {
    infrared_read(&ir);
    printf("[IR] analog_raw: %4d | gray_value: %.2f | line_detected: %d || [ULTRASONIC] CM: %.2f\n",
           ir.analog_raw, ir.gray_value, ir.line_detected, ultrasonic_get_cm());

    vTaskDelay(pdMS_TO_TICKS(1000));
  }

  //RobotMode mode = menu_get_mode();

  switch (1) {  // mode
    case 1:     //MODE_LINEARFOLLOWER
      //linear_follower_run();
      printf("Running linear follower mode\n");
      break;
    case 2:  //MODE_OBSTACLEAVOIDER
      //obstacle_avoider_run();
      printf("Running obstacle avoider mode\n");
      break;
    case 3:  //MODE_WIFI
      //wifi_run();
      printf("Running wifi mode\n");
      break;
    default:
      printf("ERROR Unknown mode\n");
  }
}