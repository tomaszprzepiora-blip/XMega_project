#include "uart_debug.h"

#include <stddef.h>

/*
 * Debug console over the on-board FT232RL USB-UART, so debug text can be
 * read in a serial terminal on the PC instead of squeezed onto the LCD.
 *
 * USARTF0: RXD0=PF2, TXD0=PF3 (ATxmega128A1 datasheet Table 30-6; PF0/PF1
 * are TWIF's SDA/SCL, not USART pins). Confirmed against the vendor's own
 * docs/xMega/mikroC PRO for AVR/UART/Uart.c example, which drives the
 * on-board USB-UART module via UARTF0_Init - i.e. USARTF0.
 */
#if defined(__AVR_XMEGA__)
#include <avr/io.h>

void uart_debug_init(uint32_t baud)
{
    uint16_t bsel = (uint16_t)((F_CPU / (16UL * baud)) - 1UL);

    PORTF.DIRCLR = (1u << 2);
    PORTF.DIRSET = (1u << 3);
    USARTF0.BAUDCTRLA = (uint8_t)bsel;
    USARTF0.BAUDCTRLB = (uint8_t)(bsel >> 8);
    USARTF0.CTRLC = USART_CHSIZE_8BIT_gc;
    USARTF0.CTRLB = USART_TXEN_bm;
}

void uart_debug_send(const char *text)
{
    while (text != NULL && *text != '\0') {
        while ((USARTF0.STATUS & USART_DREIF_bm) == 0) {
        }
        USARTF0.DATA = (uint8_t)*text++;
    }
}

#else

void uart_debug_init(uint32_t baud)
{
    (void)baud;
}

void uart_debug_send(const char *text)
{
    (void)text;
}

#endif
