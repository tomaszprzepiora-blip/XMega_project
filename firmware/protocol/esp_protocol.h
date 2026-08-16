#ifndef ESP_PROTOCOL_H
#define ESP_PROTOCOL_H

#include <stdint.h>

typedef enum {
    ESP_MSG_NONE = 0,
    ESP_MSG_OK,
    ESP_MSG_ERROR,
    ESP_MSG_STATE,
    ESP_MSG_TEMP
} esp_message_type_t;

typedef struct {
    esp_message_type_t type;
    uint8_t id;
    int16_t temp_deci_c;
    uint8_t state_on;
} esp_message_t;

uint8_t esp_protocol_parse_line(const char *line, esp_message_t *message);
void esp_protocol_button_command(uint8_t id, char *buffer, uint8_t buffer_size);
void esp_protocol_temp_command(uint8_t id, char *buffer, uint8_t buffer_size);
void esp_protocol_sync_command(char *buffer, uint8_t buffer_size);

#endif

