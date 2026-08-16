#include "adxl345.h"
#include "i2c_twid.h"

#include <stddef.h>

/*
 * mikromedia for XMEGA has an on-board ADXL345 3-axis accelerometer on the
 * TWID (I2C) bus. The 7-bit address here (0x1D) matches the vendor's own
 * demo (docs/xMega/mikroC PRO for AVR/Accel/ACCEL_driver.c), which addresses
 * it as 0x3A/0x3B (write/read octets) - i.e. ALT_ADDRESS pulled high on this
 * board, not the ADXL345 default 0x53.
 */
#define ADXL345_ADDR            0x1D
#define ADXL345_REG_DEVID       0x00
#define ADXL345_REG_BW_RATE     0x2C
#define ADXL345_REG_POWER_CTL   0x2D
#define ADXL345_REG_DATA_FORMAT 0x31
#define ADXL345_REG_DATAX0      0x32
#define ADXL345_REG_FIFO_CTL    0x38
#define ADXL345_DEVID_EXPECTED  0xE5

static uint8_t write_reg(uint8_t reg, uint8_t value)
{
    uint8_t buf[2];

    buf[0] = reg;
    buf[1] = value;
    return i2c_twid_write(ADXL345_ADDR, buf, 2);
}

static uint8_t read_regs(uint8_t reg, uint8_t *data, uint8_t len)
{
    if (!i2c_twid_write(ADXL345_ADDR, &reg, 1)) {
        return 0;
    }
    return i2c_twid_read(ADXL345_ADDR, data, len);
}

uint8_t adxl345_init(void)
{
    uint8_t devid = 0;

    i2c_twid_init();

    if (!read_regs(ADXL345_REG_DEVID, &devid, 1) || devid != ADXL345_DEVID_EXPECTED) {
        return 0;
    }

    write_reg(ADXL345_REG_DATA_FORMAT, 0x08); /* full resolution, +/-2g, 4mg/LSB */
    write_reg(ADXL345_REG_BW_RATE, 0x0A);     /* 100 Hz output data rate */
    write_reg(ADXL345_REG_FIFO_CTL, 0x00);    /* bypass FIFO, read live samples */
    write_reg(ADXL345_REG_POWER_CTL, 0x08);   /* measurement mode */
    return 1;
}

uint8_t adxl345_read(accel_sample_t *sample)
{
    uint8_t raw[6];

    if (sample == NULL) {
        return 0;
    }
    if (!read_regs(ADXL345_REG_DATAX0, raw, 6)) {
        return 0;
    }

    sample->x = (int16_t)((raw[1] << 8) | raw[0]);
    sample->y = (int16_t)((raw[3] << 8) | raw[2]);
    sample->z = (int16_t)((raw[5] << 8) | raw[4]);
    return 1;
}
