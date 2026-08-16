#ifndef UART_ESP_H
#define UART_ESP_H

#include <stdint.h>

void uart_esp_init(uint32_t baud);
void uart_esp_send(const char *text);
uint8_t uart_esp_read_line(char *buffer, uint8_t buffer_size);

#endif

