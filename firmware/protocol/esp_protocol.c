#include "esp_protocol.h"

#include <stdio.h>
#include <string.h>

static int parse_int(const char **cursor)
{
    int value = 0;
    int sign = 1;
    const char *p = *cursor;

    if (*p == '-') {
        sign = -1;
        p++;
    }

    while (*p >= '0' && *p <= '9') {
        value = (value * 10) + (*p - '0');
        p++;
    }

    *cursor = p;
    return value * sign;
}

uint8_t esp_protocol_parse_line(const char *line, esp_message_t *message)
{
    const char *p;

    if (line == NULL || message == NULL) {
        return 0;
    }

    message->type = ESP_MSG_NONE;
    message->idx = 0;
    message->value = 0;

    if (strncmp(line, "idx:", 4) == 0) {
        p = line + 4;
        if (*p < '0' || *p > '9') {
            return 0;
        }
        message->idx = (uint16_t)parse_int(&p);
        if (*p != ':') {
            return 0;
        }
        p++;
        if (*p != '-' && (*p < '0' || *p > '9')) {
            return 0;
        }
        message->value = (int16_t)parse_int(&p);
        message->type = ESP_MSG_IDX_VALUE;
        return 1;
    }

    if (strncmp(line, "RCV,", 4) == 0) {
        p = line + 4;
        message->idx = (uint16_t)parse_int(&p);
        message->type = ESP_MSG_RECEIVED;
        return 1;
    }

    if (strncmp(line, "ERR,IDX,", 8) == 0) {
        p = line + 8;
        message->idx = (uint16_t)parse_int(&p);
        message->type = ESP_MSG_ERROR;
        return 1;
    }

    if (strncmp(line, "ERR", 3) == 0) {
        message->type = ESP_MSG_ERROR;
        return 1;
    }

    if (strncmp(line, "OK", 2) == 0) {
        message->type = ESP_MSG_OK;
        return 1;
    }

    return 0;
}

void esp_protocol_set_command(uint16_t idx, uint8_t on, char *buffer, uint8_t buffer_size)
{
    (void)snprintf(buffer, buffer_size, "idx:%u:%u\n", idx, on ? 1U : 0U);
}

void esp_protocol_query_command(uint16_t idx, char *buffer, uint8_t buffer_size)
{
    (void)snprintf(buffer, buffer_size, "idx:%u:?\n", idx);
}
