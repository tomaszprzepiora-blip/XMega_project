#ifndef TOUCH_RESISTIVE_H
#define TOUCH_RESISTIVE_H

#include <stdint.h>

typedef struct {
    uint16_t x;
    uint16_t y;
} touch_point_t;

void touch_resistive_init(void);
uint8_t touch_resistive_poll(touch_point_t *point);
void touch_resistive_last_raw(uint16_t *raw_x, uint16_t *raw_y);
uint8_t touch_resistive_load_calibration(void);
uint8_t touch_resistive_calibrate(uint16_t raw_x1, uint16_t screen_x1, uint16_t raw_x2, uint16_t screen_x2,
                                   uint16_t raw_y1, uint16_t screen_y1, uint16_t raw_y2, uint16_t screen_y2);

#endif
