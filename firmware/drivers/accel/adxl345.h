#ifndef ADXL345_H
#define ADXL345_H

#include <stdint.h>

typedef struct {
    int16_t x;
    int16_t y;
    int16_t z;
} accel_sample_t;

uint8_t adxl345_init(void);
uint8_t adxl345_read(accel_sample_t *sample);

#endif
