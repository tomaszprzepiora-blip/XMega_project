#include <stdio.h>
#include <string.h>

#include "../firmware/ui/xmega_ui.h"

static char last_uart[64];

static void mock_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    (void)x;
    (void)y;
    (void)w;
    (void)h;
    (void)color;
}

static void mock_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    (void)x;
    (void)y;
    (void)w;
    (void)h;
    (void)color;
}

static void mock_draw_text(uint16_t x, uint16_t y, const char *text, uint16_t color)
{
    (void)x;
    (void)y;
    (void)text;
    (void)color;
}

static void mock_uart_send(const char *text)
{
    snprintf(last_uart, sizeof(last_uart), "%s", text);
}

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
    ui_state_t state;
    const ui_driver_t driver = {
        mock_fill_rect,
        mock_draw_rect,
        mock_draw_text,
        mock_uart_send
    };

    ui_init(&state);

    failures += require_int("outside hit", ui_hit_test(0, 0), -1);
    failures += require_int("button 1 hit", ui_hit_test(8, 102), 0);
    failures += require_int("button 9 hit", ui_hit_test(260, 200), 8);

    failures += require_int("touch button 1", ui_touch_down(&driver, &state, 8, 102), 1);
    failures += require_string("button 1 command", last_uart, "BTN,1,TOGGLE\n");
    failures += require_int("button 1 pending", state.control_state[0], UI_CONTROL_PENDING);

    failures += require_int("apply state", ui_apply_esp_line(&driver, &state, "STATE,1,ON\r\n"), 1);
    failures += require_int("button 1 on", state.control_state[0], UI_CONTROL_ON);

    failures += require_int("apply temp", ui_apply_esp_line(&driver, &state, "TEMP,2,19.8\n"), 1);
    failures += require_int("temp 2", state.temp_deci_c[1], 198);

    if (failures != 0) {
        return 1;
    }

    puts("xmega_ui tests OK");
    return 0;
}
