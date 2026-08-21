#ifndef XMEGA_UI_LAYOUT_H
#define XMEGA_UI_LAYOUT_H

#include <stdint.h>

#define UI_SCREEN_W 320
#define UI_SCREEN_H 240

#define UI_STATUS_X 0
#define UI_STATUS_Y 0
#define UI_STATUS_W 320
#define UI_STATUS_H 96

#define UI_CONTROL_COUNT 9

typedef enum {
    UI_KIND_BUTTON = 0,
    UI_KIND_SLIDER = 1
} ui_control_kind_t;

typedef struct {
    uint8_t id;
    ui_control_kind_t kind;
    uint16_t x;
    uint16_t y;
    uint16_t w;
    uint16_t h;
    const char *label;
} ui_control_t;

static const ui_control_t UI_CONTROLS[UI_CONTROL_COUNT] = {
    { 1, UI_KIND_BUTTON,  6, 100, 100, 40, "Lozko Aga" },
    { 2, UI_KIND_BUTTON,110, 100, 100, 40, "Lozko Tomek" },
    { 3, UI_KIND_BUTTON,214, 100, 100, 40, "Biurko" },

    { 4, UI_KIND_BUTTON,  6, 146, 100, 40, "Dock Station" },
    { 5, UI_KIND_BUTTON,110, 146, 100, 40, "Salon Spr." },
    { 6, UI_KIND_BUTTON,214, 146, 100, 40, "All Off" },

    { 7, UI_KIND_BUTTON,  6, 192, 100, 40, "Temp 1" },
    { 8, UI_KIND_BUTTON,110, 192, 100, 40, "Temp 2" },
    { 9, UI_KIND_BUTTON,214, 192, 100, 40, "Sync" }
};

static inline int8_t ui_hit_test(uint16_t touch_x, uint16_t touch_y)
{
    uint8_t i;

    for (i = 0; i < UI_CONTROL_COUNT; i++) {
        const ui_control_t *c = &UI_CONTROLS[i];
        if (touch_x >= c->x && touch_x < (c->x + c->w) &&
            touch_y >= c->y && touch_y < (c->y + c->h)) {
            return (int8_t)i;
        }
    }

    return -1;
}

#endif
