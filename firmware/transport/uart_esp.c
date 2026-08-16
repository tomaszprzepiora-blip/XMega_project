#include "uart_esp.h"

#include <stddef.h>

#if defined(__AVR_XMEGA__)
#include <avr/io.h>

static uint8_t rx_len;

void uart_esp_init(uint32_t baud)
{
    /*
     * USARTD0: RXD-PD2 / TXD-PD3 on mikromedia HDR2.
     * This assumes F_CPU is already set. The baud calculation is deliberately
     * conservative and may be replaced with a calibrated value once we choose
     * the final system clock.
     */
    uint16_t bsel = (uint16_t)((F_CPU / (16UL * baud)) - 1UL);

    PORTD.DIRCLR = (1u << 2);
    PORTD.DIRSET = (1u << 3);
    USARTD0.BAUDCTRLA = (uint8_t)bsel;
    USARTD0.BAUDCTRLB = (uint8_t)(bsel >> 8);
    USARTD0.CTRLC = USART_CHSIZE_8BIT_gc;
    USARTD0.CTRLB = USART_RXEN_bm | USART_TXEN_bm;
    rx_len = 0;
}

void uart_esp_send(const char *text)
{
    while (text != NULL && *text != '\0') {
        while ((USARTD0.STATUS & USART_DREIF_bm) == 0) {
        }
        USARTD0.DATA = (uint8_t)*text++;
    }
}

uint8_t uart_esp_read_line(char *buffer, uint8_t buffer_size)
{
    if (buffer == NULL || buffer_size == 0) {
        return 0;
    }

    while ((USARTD0.STATUS & USART_RXCIF_bm) != 0) {
        char ch = (char)USARTD0.DATA;
        if (ch == '\n') {
            buffer[rx_len] = '\0';
            rx_len = 0;
            return 1;
        }
        if (ch != '\r' && rx_len < buffer_size - 1) {
            buffer[rx_len++] = ch;
        }
    }

    return 0;
}

#else

void uart_esp_init(uint32_t baud)
{
    (void)baud;
}

void uart_esp_send(const char *text)
{
    (void)text;
}

uint8_t uart_esp_read_line(char *buffer, uint8_t buffer_size)
{
    (void)buffer;
    (void)buffer_size;
    return 0;
}

#endif

