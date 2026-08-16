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

static int16_t parse_temp_deci_c(const char *text)
{
    int whole;
    int frac = 0;
    const char *p = text;

    whole = parse_int(&p);
    if (*p == '.') {
        p++;
        if (*p >= '0' && *p <= '9') {
            frac = *p - '0';
        }
    }

    return (int16_t)(whole < 0 ? (whole * 10) - frac : (whole * 10) + frac);
}

static uint8_t token_equals(const char *text, const char *token)
{
    while (*token != '\0') {
        if (*text != *token) {
            return 0;
        }
        text++;
        token++;
    }

    return *text == '\0' || *text == '\r' || *text == '\n';
}

uint8_t esp_protocol_parse_line(const char *line, esp_message_t *message)
{
    const char *value;

    if (line == NULL || message == NULL) {
        return 0;
    }

    message->type = ESP_MSG_NONE;
    message->id = 0;
    message->temp_deci_c = 0;
    message->state_on = 0;

    if (strncmp(line, "TEMP,", 5) == 0) {
        value = strchr(line + 5, ',');
        if (value == NULL || line[5] < '1' || line[5] > '9') {
            return 0;
        }
        message->type = ESP_MSG_TEMP;
        message->id = (uint8_t)(line[5] - '0');
        message->temp_deci_c = parse_temp_deci_c(value + 1);
        return 1;
    }

    if (strncmp(line, "STATE,", 6) == 0) {
        value = strchr(line + 6, ',');
        if (value == NULL || line[6] < '1' || line[6] > '9') {
            return 0;
        }
        message->type = ESP_MSG_STATE;
        message->id = (uint8_t)(line[6] - '0');
        message->state_on = token_equals(value + 1, "ON");
        return 1;
    }

    if (strncmp(line, "OK", 2) == 0) {
        message->type = ESP_MSG_OK;
        return 1;
    }

    if (strncmp(line, "ERR", 3) == 0) {
        message->type = ESP_MSG_ERROR;
        return 1;
    }

    return 0;
}

void esp_protocol_button_command(uint8_t id, char *buffer, uint8_t buffer_size)
{
    const char *action = id == 6 ? "OFF_ALL" : "TOGGLE";
    (void)snprintf(buffer, buffer_size, "BTN,%u,%s\n", id, action);
}

void esp_protocol_temp_command(uint8_t id, char *buffer, uint8_t buffer_size)
{
    (void)snprintf(buffer, buffer_size, "GET,TEMP,%u\n", id);
}

void esp_protocol_sync_command(char *buffer, uint8_t buffer_size)
{
    (void)snprintf(buffer, buffer_size, "GET,ALL\n");
}

