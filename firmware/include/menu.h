#ifndef MENU_H__
#define MENU_H__

typedef enum {
  MODE_LINEAR_FOLLOWER  = 1,
  MODE_OBSTACLE_AVOIDER = 2,
  MODE_WIFI             = 3,
} RobotMode;

RobotMode menu_get_mode(void);

#endif  // MENU_H__