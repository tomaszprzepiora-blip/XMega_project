#ifndef I2C_TWID_H
#define I2C_TWID_H

#include <stdint.h>

void i2c_twid_init(void);
uint8_t i2c_twid_write(uint8_t addr7, const uint8_t *data, uint8_t len);
uint8_t i2c_twid_read(uint8_t addr7, uint8_t *data, uint8_t len);

#endif
