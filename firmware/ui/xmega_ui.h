#ifndef XMEGA_UI_H
#define XMEGA_UI_H

#include <stdint.h>
#include "xmega_ui_layout.h"

typedef enum {
    UI_CONTROL_OFF = 0,
    UI_CONTROL_ON = 1,
    UI_CONTROL_PENDING = 2,
    UI_CONTROL_ERROR = 3
} ui_control_state_t;

typedef struct {
    int16_t temp_deci_c[3];
    ui_control_state_t control_state[UI_CONTROL_COUNT];
    char status[32];
} ui_state_t;

typedef struct {
    void (*fill_rect)(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
    void (*draw_rect)(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
    void (*draw_text)(uint16_t x, uint16_t y, const char *text, uint16_t color);
    void (*uart_send)(const char *text);
} ui_driver_t;

void ui_init(ui_state_t *state);
void ui_draw_full(const ui_driver_t *driver, const ui_state_t *state);
void ui_draw_status(const ui_driver_t *driver, const ui_state_t *state);
void ui_draw_control(const ui_driver_t *driver, const ui_state_t *state, uint8_t index);
uint8_t ui_touch_down(const ui_driver_t *driver, ui_state_t *state, uint16_t x, uint16_t y);
uint8_t ui_apply_esp_line(const ui_driver_t *driver, ui_state_t *state, const char *line);

#endif

