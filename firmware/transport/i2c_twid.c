#include "i2c_twid.h"

/*
 * TWID = Two-Wire Interface on Port D on ATxmega128A1: SDA is PD0, SCL is
 * PD1 (fixed by silicon, not routable). This does not conflict with the ESP
 * UART on PD2/PD3 (USARTD0) used by uart_esp.c.
 */

#if defined(__AVR_XMEGA__)
#include <avr/io.h>

#define I2C_TWID_BAUD ((uint8_t)((F_CPU / (2UL * 100000UL)) - 5UL))

static uint8_t twid_wait(void)
{
    while ((TWID.MASTER.STATUS & (TWI_MASTER_WIF_bm | TWI_MASTER_RIF_bm)) == 0) {
    }
    return TWID.MASTER.STATUS;
}

void i2c_twid_init(void)
{
    TWID.MASTER.BAUD = I2C_TWID_BAUD;
    TWID.MASTER.CTRLA = TWI_MASTER_ENABLE_bm;
    TWID.MASTER.STATUS = TWI_MASTER_BUSSTATE_IDLE_gc;
}

uint8_t i2c_twid_write(uint8_t addr7, const uint8_t *data, uint8_t len)
{
    uint8_t i;

    TWID.MASTER.ADDR = (uint8_t)(addr7 << 1);
    twid_wait();
    if (TWID.MASTER.STATUS & TWI_MASTER_RXACK_bm) {
        TWID.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
        return 0;
    }

    for (i = 0; i < len; i++) {
        TWID.MASTER.DATA = data[i];
        twid_wait();
        if (TWID.MASTER.STATUS & TWI_MASTER_RXACK_bm) {
            TWID.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
            return 0;
        }
    }

    TWID.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
    return 1;
}

uint8_t i2c_twid_read(uint8_t addr7, uint8_t *data, uint8_t len)
{
    uint8_t i;

    if (len == 0) {
        return 0;
    }

    TWID.MASTER.ADDR = (uint8_t)((addr7 << 1) | 1u);
    twid_wait();
    if (TWID.MASTER.STATUS & TWI_MASTER_RXACK_bm) {
        TWID.MASTER.CTRLC = TWI_MASTER_CMD_STOP_gc;
        return 0;
    }

    for (i = 0; i < len; i++) {
        if (i > 0) {
            twid_wait();
        }
        data[i] = TWID.MASTER.DATA;
        if (i == (uint8_t)(len - 1)) {
            TWID.MASTER.CTRLC = TWI_MASTER_ACKACT_bm | TWI_MASTER_CMD_STOP_gc;
        } else {
            TWID.MASTER.CTRLC = TWI_MASTER_CMD_RECVTRANS_gc;
        }
    }

    return 1;
}

#else

void i2c_twid_init(void)
{
}

uint8_t i2c_twid_write(uint8_t addr7, const uint8_t *data, uint8_t len)
{
    (void)addr7;
    (void)data;
    (void)len;
    return 0;
}

uint8_t i2c_twid_read(uint8_t addr7, uint8_t *data, uint8_t len)
{
    (void)addr7;
    (void)data;
    (void)len;
    return 0;
}

#endif
