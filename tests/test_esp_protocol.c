#include <stdio.h>
#include <string.h>

#include "../firmware/protocol/esp_protocol.h"

static int require_int(const char *name, int actual, int expected)
{
    if (actual != expected) {
        fprintf(stderr, "%s: got %d, expected %d\n", name, actual, expected);
        return 1;
    }
    return 0;
}

static int require_string(const char *name, const char *actual, const char *expected)
{
    if (strcmp(actual, expected) != 0) {
        fprintf(stderr, "%s: got '%s', expected '%s'\n", name, actual, expected);
        return 1;
    }
    return 0;
}

int main(void)
{
    int failures = 0;
    char command[24];
    esp_message_t message;

    failures += require_int("parse temp idx", esp_protocol_parse_line("idx:40:216\r\n", &message), 1);
    failures += require_int("temp idx type", message.type, ESP_MSG_IDX_VALUE);
    failures += require_int("temp idx idx", message.idx, 40);
    failures += require_int("temp idx value", message.value, 216);

    failures += require_int("parse switch idx", esp_protocol_parse_line("idx:32:1\n", &message), 1);
    failures += require_int("switch idx type", message.type, ESP_MSG_IDX_VALUE);
    failures += require_int("switch idx idx", message.idx, 32);
    failures += require_int("switch idx value", message.value, 1);

    failures += require_int("parse negative idx", esp_protocol_parse_line("idx:42:-5\n", &message), 1);
    failures += require_int("negative idx value", message.value, -5);

    failures += require_int("parse rcv", esp_protocol_parse_line("RCV,32\r\n", &message), 1);
    failures += require_int("rcv type", message.type, ESP_MSG_RECEIVED);
    failures += require_int("rcv idx", message.idx, 32);

    failures += require_int("parse err idx", esp_protocol_parse_line("ERR,IDX,32\n", &message), 1);
    failures += require_int("err idx type", message.type, ESP_MSG_ERROR);
    failures += require_int("err idx idx", message.idx, 32);

    failures += require_int("parse err wifi", esp_protocol_parse_line("ERR,WIFI\n", &message), 1);
    failures += require_int("err wifi type", message.type, ESP_MSG_ERROR);

    failures += require_int("parse ok pong", esp_protocol_parse_line("OK,PONG,WIFI\n", &message), 1);
    failures += require_int("ok pong type", message.type, ESP_MSG_OK);

    esp_protocol_set_command(32, 1, command, sizeof(command));
    failures += require_string("set command on", command, "idx:32:1\n");

    esp_protocol_set_command(32, 0, command, sizeof(command));
    failures += require_string("set command off", command, "idx:32:0\n");

    esp_protocol_query_command(40, command, sizeof(command));
    failures += require_string("query command", command, "idx:40:?\n");

    if (failures != 0) {
        return 1;
    }

    puts("esp_protocol tests OK");
    return 0;
}
