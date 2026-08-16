#ifndef UART_DEBUG_H
#define UART_DEBUG_H

#include <stdint.h>

void uart_debug_init(uint32_t baud);
void uart_debug_send(const char *text);

#endif
