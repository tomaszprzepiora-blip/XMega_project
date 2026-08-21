#ifndef ESP_PROTOCOL_H
#define ESP_PROTOCOL_H

#include <stdint.h>

typedef enum {
    ESP_MSG_NONE = 0,
    ESP_MSG_OK,
    ESP_MSG_ERROR,
    ESP_MSG_RECEIVED,
    ESP_MSG_IDX_VALUE
} esp_message_type_t;

typedef struct {
    esp_message_type_t type;
    uint16_t idx;
    int16_t value;
} esp_message_t;

uint8_t esp_protocol_parse_line(const char *line, esp_message_t *message);
void esp_protocol_set_command(uint16_t idx, uint8_t on, char *buffer, uint8_t buffer_size);
void esp_protocol_query_command(uint16_t idx, char *buffer, uint8_t buffer_size);

#endif
