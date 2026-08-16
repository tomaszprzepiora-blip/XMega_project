#include "xmega_menu.h"
#include "touch_resistive.h"
#include "uart_debug.h"

#include <stddef.h>
#include <stdio.h>

#if defined(__AVR_XMEGA__)
#include <util/delay.h>
#endif

#define MENU_COLOR_BG    0x0000
#define MENU_COLOR_ITEM  0x2965
#define MENU_COLOR_LINE  0x4208
#define MENU_COLOR_TEXT  0xFFFF

#define MENU_ITEM_X 20
#define MENU_ITEM_W (UI_SCREEN_W - 2 * MENU_ITEM_X)
#define MENU_ITEM_H 44
#define MENU_ITEM_GAP 4
#define MENU_TOP_Y 40

/*
 * Calibration walks the user through the CENTER of every one of the 9 real
 * buttons, by name ("Dotknij: Salon"), one at a time - not just 2 or 4
 * abstract/corner points. Every button gets its own on-screen prompt and
 * its own captured sample, so there is nothing calculated "behind the
 * scenes" the user cannot see confirmed.
 *
 * The underlying model is still the same 2-point-per-axis linear map used
 * by touch_resistive_calibrate() (X from the left column vs the right
 * column, Y from the top row vs the bottom row), but each side now comes
 * from averaging 3 real button taps instead of 1-2, and every column/row
 * triplet is checked for mutual agreement before being trusted - so a
 * single mis-tap on any one of the 9 buttons is caught and retried rather
 * than silently skewing the result (which is what happened with the
 * previous 4-corner version: a bad "Sync" tap passed the old pairwise
 * check but still produced a calibration where only the top row worked).
 */
#define CAL_LEFT_COL   0U /* Salon */
#define CAL_MID_COL    1U /* Kuchnia */
#define CAL_RIGHT_COL  2U /* Korytarz */
#define CAL_TOP_ROW    0U /* Salon */
#define CAL_MID_ROW    1U /* Biurko */
#define CAL_BOTTOM_ROW 2U /* Temp 1 */

/* UI_CONTROLS index for grid position (col, row), both 0..2. */
static uint8_t cal_control_index(uint8_t col, uint8_t row)
{
    return (uint8_t)(row * 3U + col);
}

static uint16_t cal_center_x(uint8_t control_index)
{
    const ui_control_t *c = &UI_CONTROLS[control_index];
    return (uint16_t)(c->x + c->w / 2U);
}

static uint16_t cal_center_y(uint8_t control_index)
{
    const ui_control_t *c = &UI_CONTROLS[control_index];
    return (uint16_t)(c->y + c->h / 2U);
}

static void draw_crosshair(const ui_driver_t *driver, uint16_t x, uint16_t y, uint16_t color)
{
    driver->fill_rect((uint16_t)(x - 6), y, 13, 1, color);
    driver->fill_rect(x, (uint16_t)(y - 6), 1, 13, color);
}

static uint8_t wait_for_touch_raw(uint16_t *raw_x, uint16_t *raw_y)
{
    touch_point_t p;

#if defined(__AVR_XMEGA__)
    uint16_t guard;

    for (guard = 0; guard < 3000U; guard++) {
        if (touch_resistive_poll(&p)) {
            touch_resistive_last_raw(raw_x, raw_y);
            return 1;
        }
        _delay_ms(10);
    }
    return 0;
#else
    (void)p;
    (void)raw_x;
    (void)raw_y;
    return 0;
#endif
}

#define CAL_MAX_ATTEMPTS 5U
#define CAL_GROUP_TOLERANCE 300U

static uint8_t prompt_and_capture(const ui_driver_t *driver, uint8_t control_index,
                                   uint16_t *raw_x, uint16_t *raw_y)
{
    char line[48];
    uint16_t tx = cal_center_x(control_index);
    uint16_t ty = cal_center_y(control_index);

    driver->fill_rect(0, 0, UI_SCREEN_W, UI_SCREEN_H, MENU_COLOR_BG);
    driver->draw_text(20, 40, "Dotknij przycisk:", MENU_COLOR_TEXT);
    driver->draw_text(20, 60, UI_CONTROLS[control_index].label, MENU_COLOR_TEXT);
    draw_crosshair(driver, tx, ty, MENU_COLOR_TEXT);

    if (!wait_for_touch_raw(raw_x, raw_y)) {
        uart_debug_send("cal: timeout\r\n");
        return 0;
    }

    (void)snprintf(line, sizeof(line), "cal: %s raw=%u,%u center=%u,%u\r\n",
                   UI_CONTROLS[control_index].label, *raw_x, *raw_y, tx, ty);
    uart_debug_send(line);

#if defined(__AVR_XMEGA__)
    /* Let the finger lift before the next target is shown. */
    _delay_ms(400);
#endif
    return 1;
}

/* Largest pairwise difference among a small group of raw samples. */
static uint16_t cal_group_spread(const uint16_t *values, uint8_t count)
{
    uint16_t max_v = values[0];
    uint16_t min_v = values[0];
    uint8_t i;

    for (i = 1; i < count; i++) {
        if (values[i] > max_v) {
            max_v = values[i];
        }
        if (values[i] < min_v) {
            min_v = values[i];
        }
    }
    return (uint16_t)(max_v - min_v);
}

static uint8_t check_group(const char *name, const uint16_t *values, uint8_t count)
{
    uint16_t spread = cal_group_spread(values, count);
    char line[48];

    if (spread <= CAL_GROUP_TOLERANCE) {
        return 1;
    }
    (void)snprintf(line, sizeof(line), "cal: %s disagrees, spread=%u\r\n", name, spread);
    uart_debug_send(line);
    return 0;
}

static void run_calibration(const ui_driver_t *driver)
{
    /* raw_x[col][row], raw_y[col][row] - one sample per button, indexed by
     * grid position so column/row groups are easy to pull out below. */
    uint16_t raw_x[3][3];
    uint16_t raw_y[3][3];
    uint8_t attempt;

    for (attempt = 0; attempt < CAL_MAX_ATTEMPTS; attempt++) {
        uint8_t ok = 1;
        uint8_t col;
        uint8_t row;
        uint16_t group[3];
        uint32_t sum;
        uint16_t x_left;
        uint16_t x_right;
        uint16_t y_top;
        uint16_t y_bottom;

        for (row = 0; row < 3U && ok; row++) {
            for (col = 0; col < 3U && ok; col++) {
                ok = prompt_and_capture(driver, cal_control_index(col, row),
                                        &raw_x[col][row], &raw_y[col][row]);
            }
        }
        if (!ok) {
            return;
        }

        group[0] = raw_x[CAL_LEFT_COL][0];
        group[1] = raw_x[CAL_LEFT_COL][1];
        group[2] = raw_x[CAL_LEFT_COL][2];
        ok = check_group("lewa kolumna", group, 3) && ok;
        sum = (uint32_t)group[0] + group[1] + group[2];
        x_left = (uint16_t)(sum / 3U);

        group[0] = raw_x[CAL_RIGHT_COL][0];
        group[1] = raw_x[CAL_RIGHT_COL][1];
        group[2] = raw_x[CAL_RIGHT_COL][2];
        ok = check_group("prawa kolumna", group, 3) && ok;
        sum = (uint32_t)group[0] + group[1] + group[2];
        x_right = (uint16_t)(sum / 3U);

        group[0] = raw_y[0][CAL_TOP_ROW];
        group[1] = raw_y[1][CAL_TOP_ROW];
        group[2] = raw_y[2][CAL_TOP_ROW];
        ok = check_group("gorny rzad", group, 3) && ok;
        sum = (uint32_t)group[0] + group[1] + group[2];
        y_top = (uint16_t)(sum / 3U);

        group[0] = raw_y[0][CAL_BOTTOM_ROW];
        group[1] = raw_y[1][CAL_BOTTOM_ROW];
        group[2] = raw_y[2][CAL_BOTTOM_ROW];
        ok = check_group("dolny rzad", group, 3) && ok;
        sum = (uint32_t)group[0] + group[1] + group[2];
        y_bottom = (uint16_t)(sum / 3U);

        if (ok && touch_resistive_calibrate(x_left, cal_center_x(cal_control_index(CAL_LEFT_COL, 0)),
                                             x_right, cal_center_x(cal_control_index(CAL_RIGHT_COL, 0)),
                                             y_top, cal_center_y(cal_control_index(0, CAL_TOP_ROW)),
                                             y_bottom, cal_center_y(cal_control_index(0, CAL_BOTTOM_ROW)))) {
            uart_debug_send("cal: saved\r\n");
            driver->fill_rect(0, 0, UI_SCREEN_W, UI_SCREEN_H, MENU_COLOR_BG);
            driver->draw_text(70, 110, "Kalibracja zapisana", MENU_COLOR_TEXT);
#if defined(__AVR_XMEGA__)
            _delay_ms(1200);
#endif
            return;
        }

        uart_debug_send("cal: rejected, retrying\r\n");
        driver->fill_rect(0, 0, UI_SCREEN_W, UI_SCREEN_H, MENU_COLOR_BG);
        driver->draw_text(30, 100, "Nieudana, sprobuj ponownie", MENU_COLOR_TEXT);
        driver->draw_text(30, 120, "(dotykaj dokladnie w srodek)", MENU_COLOR_TEXT);
#if defined(__AVR_XMEGA__)
        _delay_ms(1500);
#endif
    }

    uart_debug_send("cal: gave up after max attempts\r\n");
    driver->fill_rect(0, 0, UI_SCREEN_W, UI_SCREEN_H, MENU_COLOR_BG);
    driver->draw_text(40, 110, "Kalibracja nieudana, wyjscie", MENU_COLOR_TEXT);
#if defined(__AVR_XMEGA__)
    _delay_ms(1500);
#endif
}

typedef struct {
    const char *label;
    void (*action)(const ui_driver_t *driver);
} menu_item_t;

/*
 * Add future menu entries here (label + handler).
 *
 * menu_run() deliberately does NOT hit-test rows by calibrated screen
 * position: this menu is also how a bad/missing touch calibration gets
 * fixed, so its own navigation cannot depend on calibration already being
 * good (that was the bug: tapping "Kalibracja dotyku" could land on "Wyjdz"
 * instead because touch.y was wrong before the fix even ran). For now, with
 * a single real item, interaction is calibration-free: a short tap anywhere
 * runs MENU_ITEMS[0]; a long press (~1.4s) anywhere exits without running
 * anything. If a second item is ever added, this needs a calibration-free
 * selection scheme too (e.g. tap to cycle + long-press to confirm) rather
 * than reintroducing per-row spatial hit-testing.
 */
static const menu_item_t MENU_ITEMS[] = {
    { "Kalibracja dotyku", run_calibration },
};

#define MENU_EXIT_HOLD_TICKS 8U

static void draw_menu(const ui_driver_t *driver)
{
    driver->fill_rect(0, 0, UI_SCREEN_W, UI_SCREEN_H, MENU_COLOR_BG);
    driver->draw_text(MENU_ITEM_X, 20, "Menu", MENU_COLOR_TEXT);

    driver->fill_rect(MENU_ITEM_X, MENU_TOP_Y, MENU_ITEM_W, MENU_ITEM_H - MENU_ITEM_GAP, MENU_COLOR_ITEM);
    driver->draw_rect(MENU_ITEM_X, MENU_TOP_Y, MENU_ITEM_W, MENU_ITEM_H - MENU_ITEM_GAP, MENU_COLOR_LINE);
    driver->draw_text((uint16_t)(MENU_ITEM_X + 10), (uint16_t)(MENU_TOP_Y + 14),
                       MENU_ITEMS[0].label, MENU_COLOR_TEXT);

    driver->draw_text(MENU_ITEM_X, (uint16_t)(MENU_TOP_Y + MENU_ITEM_H + 20), "Dotknij: uruchom", MENU_COLOR_TEXT);
    driver->draw_text(MENU_ITEM_X, (uint16_t)(MENU_TOP_Y + MENU_ITEM_H + 40), "Przytrzymaj: wyjdz", MENU_COLOR_TEXT);
}

void menu_run(const ui_driver_t *driver)
{
    if (driver == NULL) {
        return;
    }

    draw_menu(driver);

#if defined(__AVR_XMEGA__)
    {
        touch_point_t p;
        uint16_t hold_ticks = 0;
        uint16_t gap_ticks = 0;

        for (;;) {
            if (touch_resistive_poll(&p)) {
                gap_ticks = 0;
                hold_ticks++;
                if (hold_ticks >= MENU_EXIT_HOLD_TICKS) {
                    uart_debug_send("menu: long press, exit\r\n");
                    return;
                }
            } else if (hold_ticks > 0 && ++gap_ticks > 40U) {
                uart_debug_send("menu: short tap, running item 0\r\n");
                MENU_ITEMS[0].action(driver);
                return;
            }
            _delay_ms(10);
        }
    }
#else
    (void)driver;
#endif
}
