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

    failures += require_int("parse TEMP", esp_protocol_parse_line("TEMP,2,19.8\r\n", &message), 1);
    failures += require_int("TEMP type", message.type, ESP_MSG_TEMP);
    failures += require_int("TEMP id", message.id, 2);
    failures += require_int("TEMP value", message.temp_deci_c, 198);

    failures += require_int("parse STATE", esp_protocol_parse_line("STATE,1,ON\n", &message), 1);
    failures += require_int("STATE type", message.type, ESP_MSG_STATE);
    failures += require_int("STATE id", message.id, 1);
    failures += require_int("STATE on", message.state_on, 1);

    esp_protocol_button_command(6, command, sizeof(command));
    failures += require_string("button 6 command", command, "BTN,6,OFF_ALL\n");

    esp_protocol_temp_command(3, command, sizeof(command));
    failures += require_string("temp 3 command", command, "GET,TEMP,3\n");

    esp_protocol_sync_command(command, sizeof(command));
    failures += require_string("sync command", command, "GET,ALL\n");

    if (failures != 0) {
        return 1;
    }

    puts("esp_protocol tests OK");
    return 0;
}

