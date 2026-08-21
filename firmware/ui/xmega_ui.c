#include "xmega_ui.h"
#include "esp_protocol.h"
#include "domoticz_map.h"

#include <stddef.h>
#include <stdio.h>

#define UI_COLOR_BG       0x1082
#define UI_COLOR_TOP      0x18E4
#define UI_COLOR_PANEL    0x2106
#define UI_COLOR_LINE     0x4208
#define UI_COLOR_TEXT     0xFFFF
#define UI_COLOR_MUTED    0x9CF3
#define UI_COLOR_ON       0x0648
#define UI_COLOR_PENDING  0x8400
#define UI_COLOR_ERROR    0xA000

static void copy_status(ui_state_t *state, const char *text)
{
    size_t i;

    if (state == NULL || text == NULL) {
        return;
    }

    for (i = 0; i < sizeof(state->status) - 1 && text[i] != '\0'; i++) {
        state->status[i] = text[i];
    }
    state->status[i] = '\0';
}

static void format_temp(char *buffer, size_t size, int16_t temp_deci_c)
{
    int16_t whole = temp_deci_c / 10;
    int16_t frac = temp_deci_c % 10;

    if (frac < 0) {
        frac = (int16_t)-frac;
    }

    (void)snprintf(buffer, size, "%d.%dC", whole, frac);
}

static uint16_t control_fill(ui_control_state_t state)
{
    switch (state) {
    case UI_CONTROL_ON:
        return UI_COLOR_ON;
    case UI_CONTROL_PENDING:
        return UI_COLOR_PENDING;
    case UI_CONTROL_ERROR:
        return UI_COLOR_ERROR;
    case UI_CONTROL_OFF:
    default:
        return UI_COLOR_PANEL;
    }
}

void ui_init(ui_state_t *state)
{
    uint8_t i;

    if (state == NULL) {
        return;
    }

    state->temp_deci_c[0] = 216;
    state->temp_deci_c[1] = 204;
    state->temp_deci_c[2] = 148;
    copy_status(state, "ESP UART: init");
    state->pending_command[0] = '\0';
    state->pending_retryable = 0;

    for (i = 0; i < UI_CONTROL_COUNT; i++) {
        state->control_state[i] = UI_CONTROL_OFF;
    }
}

void ui_draw_status(const ui_driver_t *driver, const ui_state_t *state)
{
    char value[12];

    if (driver == NULL || state == NULL ||
        driver->fill_rect == NULL || driver->draw_text == NULL) {
        return;
    }

    driver->fill_rect(UI_STATUS_X, UI_STATUS_Y, UI_STATUS_W, UI_STATUS_H, UI_COLOR_TOP);
    driver->draw_text(8, 8, "Domoticz Panel", UI_COLOR_TEXT);
    driver->draw_text(8, 25, state->status, UI_COLOR_MUTED);

    format_temp(value, sizeof(value), state->temp_deci_c[0]);
    driver->draw_text(8, 52, "Salon", UI_COLOR_MUTED);
    driver->draw_text(8, 66, value, UI_COLOR_TEXT);

    format_temp(value, sizeof(value), state->temp_deci_c[1]);
    driver->draw_text(112, 52, "Sypialnia", UI_COLOR_MUTED);
    driver->draw_text(112, 66, value, UI_COLOR_TEXT);

    format_temp(value, sizeof(value), state->temp_deci_c[2]);
    driver->draw_text(216, 52, "Zewn.", UI_COLOR_MUTED);
    driver->draw_text(216, 66, value, UI_COLOR_TEXT);
}

void ui_draw_control(const ui_driver_t *driver, const ui_state_t *state, uint8_t index)
{
    const ui_control_t *control;
    uint16_t fill;

    if (driver == NULL || state == NULL || index >= UI_CONTROL_COUNT ||
        driver->fill_rect == NULL || driver->draw_rect == NULL || driver->draw_text == NULL) {
        return;
    }

    control = &UI_CONTROLS[index];
    fill = control_fill(state->control_state[index]);

    driver->fill_rect(control->x, control->y, control->w, control->h, fill);
    driver->draw_rect(control->x, control->y, control->w, control->h, UI_COLOR_LINE);
    driver->draw_text((uint16_t)(control->x + 8), (uint16_t)(control->y + 14), control->label, UI_COLOR_TEXT);
}

void ui_draw_full(const ui_driver_t *driver, const ui_state_t *state)
{
    uint8_t i;

    if (driver == NULL || state == NULL || driver->fill_rect == NULL) {
        return;
    }

    driver->fill_rect(0, 0, UI_SCREEN_W, UI_SCREEN_H, UI_COLOR_BG);
    ui_draw_status(driver, state);

    for (i = 0; i < UI_CONTROL_COUNT; i++) {
        ui_draw_control(driver, state, i);
    }
}

uint8_t ui_touch_down(const ui_driver_t *driver, ui_state_t *state, uint16_t x, uint16_t y)
{
    char command[24];
    int8_t hit;
    uint8_t id;
    uint8_t i;

    if (state == NULL) {
        return 0;
    }

    hit = ui_hit_test(x, y);
    if (hit < 0) {
        return 0;
    }

    id = UI_CONTROLS[hit].id;
    copy_status(state, "UART -> ESP");

    if (id <= 5) {
        /*
         * Local optimistic toggle: press when off -> shows as pending-on;
         * press again (whether still pending or already confirmed on by an
         * ESP idx message) -> goes back off. Without this, every press
         * just set the same "on" state again, so a lit button could never
         * be turned back off from the panel itself.
         */
        uint8_t new_on = (state->control_state[hit] == UI_CONTROL_OFF) ? 1 : 0;
        state->control_state[hit] = new_on ? UI_CONTROL_PENDING : UI_CONTROL_OFF;
        state->pending_retryable = 1;
        if (driver != NULL && driver->uart_send != NULL) {
            esp_protocol_set_command(UI_SWITCH_IDX[hit], new_on, state->pending_command, sizeof(state->pending_command));
            driver->uart_send(state->pending_command);
        }
    } else if (id == 6) {
        /* All Off: turn off every known switch, one idx command per line.
         * Not retried - see pending_retryable comment in xmega_ui.h. */
        state->pending_retryable = 0;
        for (i = 0; i < UI_SWITCH_COUNT; i++) {
            state->control_state[i] = UI_CONTROL_OFF;
            if (driver != NULL && driver->uart_send != NULL) {
                esp_protocol_set_command(UI_SWITCH_IDX[i], 0, command, sizeof(command));
                driver->uart_send(command);
            }
            ui_draw_control(driver, state, i);
        }
    } else if (id <= 8) {
        state->pending_retryable = 1;
        if (driver != NULL && driver->uart_send != NULL) {
            esp_protocol_query_command(UI_TEMP_IDX[id - 7], state->pending_command, sizeof(state->pending_command));
            driver->uart_send(state->pending_command);
        }
    } else {
        /* Sync: refresh every known switch and sensor. Not retried. */
        state->pending_retryable = 0;
        if (driver != NULL && driver->uart_send != NULL) {
            for (i = 0; i < UI_SWITCH_COUNT; i++) {
                esp_protocol_query_command(UI_SWITCH_IDX[i], command, sizeof(command));
                driver->uart_send(command);
            }
            for (i = 0; i < UI_TEMP_COUNT; i++) {
                esp_protocol_query_command(UI_TEMP_IDX[i], command, sizeof(command));
                driver->uart_send(command);
            }
        }
    }

    ui_draw_status(driver, state);
    ui_draw_control(driver, state, (uint8_t)hit);

    return 1;
}

uint8_t ui_apply_esp_line(const ui_driver_t *driver, ui_state_t *state, const char *line)
{
    esp_message_t message;
    uint8_t i;

    if (state == NULL || line == NULL) {
        return 0;
    }

    if (!esp_protocol_parse_line(line, &message)) {
        return 0;
    }

    switch (message.type) {
    case ESP_MSG_IDX_VALUE:
        for (i = 0; i < UI_SWITCH_COUNT; i++) {
            if (UI_SWITCH_IDX[i] == message.idx) {
                state->control_state[i] = message.value ? UI_CONTROL_ON : UI_CONTROL_OFF;
                copy_status(state, "STATE update");
                ui_draw_status(driver, state);
                ui_draw_control(driver, state, i);
                return 1;
            }
        }
        for (i = 0; i < UI_TEMP_COUNT; i++) {
            if (UI_TEMP_IDX[i] == message.idx) {
                state->temp_deci_c[i] = message.value;
                copy_status(state, "TEMP update");
                ui_draw_status(driver, state);
                return 1;
            }
        }
        break;

    case ESP_MSG_RECEIVED:
        copy_status(state, "ESP: PRZYJETO");
        ui_draw_status(driver, state);
        return 2;

    case ESP_MSG_OK:
        copy_status(state, "ESP OK");
        ui_draw_status(driver, state);
        return 1;

    case ESP_MSG_ERROR:
        for (i = 0; i < UI_SWITCH_COUNT; i++) {
            if (UI_SWITCH_IDX[i] == message.idx) {
                state->control_state[i] = UI_CONTROL_ERROR;
                ui_draw_control(driver, state, i);
                break;
            }
        }
        copy_status(state, "ESP ERR");
        ui_draw_status(driver, state);
        return 1;

    case ESP_MSG_NONE:
    default:
        break;
    }

    return 0;
}
