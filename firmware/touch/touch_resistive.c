#include "touch_resistive.h"

#if defined(__AVR_XMEGA__)
#include <avr/eeprom.h>
#include <avr/io.h>
#include <util/delay.h>
#endif

#include "xmega_ui_layout.h"

/*
 * Reader for the MI0283QT2 resistive touch panel on mikromedia for XMEGA.
 *
 * The board does NOT expose the 4 touch panel wires (XL/XR/YU/YD) directly:
 * only XL (PA2) and YD (PA1) reach the MCU as analog pins. XR and YU are
 * driven by a discrete transistor network (Q6-Q10 in the schematic) selected
 * by two digital control lines, DRIVEA (PQ2) and DRIVEB (PQ3):
 *
 *   DRIVEA=1, DRIVEB=0: XL driven LOW, XR driven HIGH -> the panel's own
 *                        "X" electrode pair is driven; YD is left
 *                        high-impedance and is the pin to sample.
 *   DRIVEA=0, DRIVEB=1: YU driven HIGH, YD driven LOW -> the panel's own
 *                        "Y" electrode pair is driven; XL is left
 *                        high-impedance and is the pin to sample.
 *
 * IMPORTANT: the touch overlay's own "X"/"Y" electrical axes (as wired on
 * the schematic) are rotated 90 degrees relative to the screen's X/Y as
 * drawn. Confirmed empirically by a 4-corner calibration sweep (see
 * xmega_menu.c) spanning the outer buttons of the 3x3 grid: taps that
 * shared the same screen Y (top row) gave near-identical readings on the
 * panel's "X" phase regardless of screen X, and taps sharing the same
 * screen X (left column) gave near-identical readings on the panel's "Y"
 * phase regardless of screen Y. So the sample taken while the "X" pair is
 * driven tracks screen Y, and the sample taken while the "Y" pair is
 * driven tracks screen X - the two are swapped below when building the
 * touch_point_t. Both axes are also electrically inverted (raw decreases
 * as the screen coordinate increases), which is why map_axis() below does
 * signed math instead of assuming raw_min < raw_max.
 *
 * The raw range varies with panel tolerance, so it is not a fixed compile
 * time constant: it is measured once by an on-screen calibration wizard
 * (see touch_resistive_calibrate()) and stored in EEPROM. The values below
 * are only the fallback used before any calibration has been saved.
 *
 * No-touch detection: with no finger present, the "X"-phase sample (from
 * YD) settles high, near 3120 out of 4095 - well above any real touch
 * observed (which topped out under 2100). The "Y"-phase sample alone
 * cannot tell touch from no-touch (its idle value overlaps real touches),
 * so TOUCH_NO_TOUCH_X_MIN gates on the "X"-phase sample specifically.
 */

/* Fallback calibration: raw value at screen coordinate 0, and at the
 * opposite edge (UI_SCREEN_W-1 / UI_SCREEN_H-1). Derived from the 4-corner
 * sweep referenced above; a successful run of the wizard overwrites these. */
#define TOUCH_RAW_X_AT_0_DEFAULT   3440U
#define TOUCH_RAW_X_AT_MAX_DEFAULT 370U
#define TOUCH_RAW_Y_AT_0_DEFAULT   2870U
#define TOUCH_RAW_Y_AT_MAX_DEFAULT 870U
#define TOUCH_NO_TOUCH_X_MIN 2700U
#define TOUCH_REPEAT_GUARD 18U
#define TOUCH_EEPROM_MAGIC 0xCA11U

static uint8_t touch_guard_ticks;
static uint16_t touch_last_raw_x;
static uint16_t touch_last_raw_y;
static uint16_t touch_raw_x_at_0 = TOUCH_RAW_X_AT_0_DEFAULT;
static uint16_t touch_raw_x_at_max = TOUCH_RAW_X_AT_MAX_DEFAULT;
static uint16_t touch_raw_y_at_0 = TOUCH_RAW_Y_AT_0_DEFAULT;
static uint16_t touch_raw_y_at_max = TOUCH_RAW_Y_AT_MAX_DEFAULT;

#if defined(__AVR_XMEGA__)
static uint16_t EEMEM eeprom_magic;
static uint16_t EEMEM eeprom_raw_x_at_0;
static uint16_t EEMEM eeprom_raw_x_at_max;
static uint16_t EEMEM eeprom_raw_y_at_0;
static uint16_t EEMEM eeprom_raw_y_at_max;
#endif

#if defined(__AVR_XMEGA__)
/*
 * screen = (raw - raw_at_0) * screen_max / (raw_at_max - raw_at_0), clamped
 * to [0, screen_max]. Signed throughout so an inverted axis (raw_at_max <
 * raw_at_0, i.e. raw decreases as the screen coordinate increases) works
 * the same as a normal one - see the file header comment.
 */
static uint16_t map_axis(uint16_t raw, uint16_t raw_at_0, uint16_t raw_at_max, uint16_t screen_max)
{
    int32_t den = (int32_t)raw_at_max - (int32_t)raw_at_0;
    int32_t result;

    if (den == 0) {
        return 0;
    }

    result = ((int32_t)raw - (int32_t)raw_at_0) * (int32_t)screen_max / den;
    if (result < 0) {
        result = 0;
    } else if (result > (int32_t)screen_max) {
        result = (int32_t)screen_max;
    }
    return (uint16_t)result;
}

static void adc_start(void)
{
    ADCA.CH0.CTRL = ADC_CH_INPUTMODE_SINGLEENDED_gc;
    ADCA.CTRLB = ADC_RESOLUTION_12BIT_gc;
    ADCA.REFCTRL = ADC_REFSEL_VCC_gc;
    ADCA.PRESCALER = ADC_PRESCALER_DIV128_gc;
    ADCA.CTRLA = ADC_ENABLE_bm;
}

static uint16_t adc_read_pin(uint8_t mux)
{
    ADCA.CH0.MUXCTRL = mux;
    ADCA.CH0.CTRL |= ADC_CH_START_bm;
    while (!(ADCA.CH0.INTFLAGS & ADC_CH_CHIF_bm)) {
    }
    ADCA.CH0.INTFLAGS = ADC_CH_CHIF_bm;
    return ADCA.CH0.RES;
}

/*
 * The sense pin's own RC filter (100K + 100nF, see the schematic notes
 * above) needs time to settle after switching DRIVEA/DRIVEB, and a single
 * ADC sample of a resistive touch panel is inherently noisy. Take a few
 * back-to-back readings and average them, discarding the very first one
 * (still settling) rather than trusting one raw sample.
 */
#define TOUCH_ADC_SAMPLES 4U

static uint16_t adc_read_avg(uint8_t mux)
{
    uint16_t i;
    uint32_t sum = 0;

    (void)adc_read_pin(mux);
    for (i = 0; i < TOUCH_ADC_SAMPLES; i++) {
        sum += adc_read_pin(mux);
    }
    return (uint16_t)(sum / TOUCH_ADC_SAMPLES);
}

static void drive_touch_x(void)
{
    /* DRIVEA=1, DRIVEB=0: drives the panel's "X" pair; sample YD (PA1). */
    PORTQ.OUTSET = PIN2_bm;
    PORTQ.OUTCLR = PIN3_bm;
    _delay_ms(2);
}

static void drive_touch_y(void)
{
    /* DRIVEA=0, DRIVEB=1: drives the panel's "Y" pair; sample XL (PA2). */
    PORTQ.OUTCLR = PIN2_bm;
    PORTQ.OUTSET = PIN3_bm;
    _delay_ms(2);
}

static void release_touch_drive(void)
{
    PORTQ.OUTCLR = PIN2_bm | PIN3_bm;
}
#endif

void touch_resistive_init(void)
{
    touch_guard_ticks = 0;

#if defined(__AVR_XMEGA__)
    PORTA.DIRCLR = PIN1_bm | PIN2_bm;
    PORTA.PIN1CTRL = PORT_OPC_TOTEM_gc;
    PORTA.PIN2CTRL = PORT_OPC_TOTEM_gc;

    PORTQ.DIRSET = PIN2_bm | PIN3_bm;
    release_touch_drive();
    adc_start();
#endif
}

uint8_t touch_resistive_poll(touch_point_t *point)
{
#if defined(__AVR_XMEGA__)
    uint16_t sample_x_phase;
    uint16_t sample_y_phase;

    if (touch_guard_ticks > 0) {
        touch_guard_ticks--;
        return 0;
    }

    drive_touch_x();
    sample_x_phase = adc_read_avg(ADC_CH_MUXPOS_PIN1_gc);
    drive_touch_y();
    sample_y_phase = adc_read_avg(ADC_CH_MUXPOS_PIN2_gc);
    release_touch_drive();

    /* See TOUCH_NO_TOUCH_X_MIN comment above: the "X"-phase sample alone
     * tells touch apart from no-touch on this panel. */
    if (sample_x_phase > TOUCH_NO_TOUCH_X_MIN) {
        return 0;
    }

    /* Swapped on purpose - see the file header comment. */
    touch_last_raw_x = sample_y_phase;
    touch_last_raw_y = sample_x_phase;
    point->x = map_axis(sample_y_phase, touch_raw_x_at_0, touch_raw_x_at_max, UI_SCREEN_W - 1U);
    point->y = map_axis(sample_x_phase, touch_raw_y_at_0, touch_raw_y_at_max, UI_SCREEN_H - 1U);
    touch_guard_ticks = TOUCH_REPEAT_GUARD;
    return 1;
#else
    (void)point;
    return 0;
#endif
}

void touch_resistive_last_raw(uint16_t *raw_x, uint16_t *raw_y)
{
    *raw_x = touch_last_raw_x;
    *raw_y = touch_last_raw_y;
}

uint8_t touch_resistive_load_calibration(void)
{
#if defined(__AVR_XMEGA__)
    uint16_t magic = eeprom_read_word(&eeprom_magic);
    uint16_t x_at_0, x_at_max, y_at_0, y_at_max;

    if (magic != TOUCH_EEPROM_MAGIC) {
        return 0;
    }

    x_at_0 = eeprom_read_word(&eeprom_raw_x_at_0);
    x_at_max = eeprom_read_word(&eeprom_raw_x_at_max);
    y_at_0 = eeprom_read_word(&eeprom_raw_y_at_0);
    y_at_max = eeprom_read_word(&eeprom_raw_y_at_max);

    /* Axes are inverted on this panel (see header comment), so do not
     * require at_0 < at_max - only that each axis has a real (nonzero)
     * spread to divide by. */
    if (x_at_0 == x_at_max || y_at_0 == y_at_max) {
        return 0;
    }

    touch_raw_x_at_0 = x_at_0;
    touch_raw_x_at_max = x_at_max;
    touch_raw_y_at_0 = y_at_0;
    touch_raw_y_at_max = y_at_max;
    return 1;
#else
    return 0;
#endif
}

/*
 * Derives calibration from two on-screen targets per axis: given two (raw,
 * screen) pairs, solve the same linear model used by map_axis() - screen =
 * (raw - raw_at_0) * screen_max / (raw_at_max - raw_at_0) - for the
 * raw_at_0/raw_at_max that would produce those exact two measured points,
 * then store and persist them. Works for both normal and inverted axes
 * (raw decreasing as screen increases) since only a nonzero raw delta is
 * required, not a particular sign.
 */
uint8_t touch_resistive_calibrate(uint16_t raw_x1, uint16_t screen_x1, uint16_t raw_x2, uint16_t screen_x2,
                                   uint16_t raw_y1, uint16_t screen_y1, uint16_t raw_y2, uint16_t screen_y2)
{
#if defined(__AVR_XMEGA__)
    int32_t dx_raw = (int32_t)raw_x2 - (int32_t)raw_x1;
    int32_t dx_screen = (int32_t)screen_x2 - (int32_t)screen_x1;
    int32_t dy_raw = (int32_t)raw_y2 - (int32_t)raw_y1;
    int32_t dy_screen = (int32_t)screen_y2 - (int32_t)screen_y1;
    int32_t x_at_0;
    int32_t x_at_max;
    int32_t y_at_0;
    int32_t y_at_max;

    if (dx_raw == 0 || dx_screen == 0 || dy_raw == 0 || dy_screen == 0) {
        return 0;
    }

    /* raw_at_0 = raw1 - screen1 * (raw2-raw1) / (screen2-screen1) */
    x_at_0 = (int32_t)raw_x1 - (int32_t)screen_x1 * dx_raw / dx_screen;
    x_at_max = x_at_0 + dx_raw * (int32_t)(UI_SCREEN_W - 1U) / dx_screen;
    y_at_0 = (int32_t)raw_y1 - (int32_t)screen_y1 * dy_raw / dy_screen;
    y_at_max = y_at_0 + dy_raw * (int32_t)(UI_SCREEN_H - 1U) / dy_screen;

    if (x_at_0 < 0 || x_at_0 > 4095 || x_at_max < 0 || x_at_max > 4095 ||
        y_at_0 < 0 || y_at_0 > 4095 || y_at_max < 0 || y_at_max > 4095 ||
        x_at_0 == x_at_max || y_at_0 == y_at_max) {
        return 0;
    }

    touch_raw_x_at_0 = (uint16_t)x_at_0;
    touch_raw_x_at_max = (uint16_t)x_at_max;
    touch_raw_y_at_0 = (uint16_t)y_at_0;
    touch_raw_y_at_max = (uint16_t)y_at_max;

    eeprom_write_word(&eeprom_raw_x_at_0, touch_raw_x_at_0);
    eeprom_write_word(&eeprom_raw_x_at_max, touch_raw_x_at_max);
    eeprom_write_word(&eeprom_raw_y_at_0, touch_raw_y_at_0);
    eeprom_write_word(&eeprom_raw_y_at_max, touch_raw_y_at_max);
    eeprom_write_word(&eeprom_magic, TOUCH_EEPROM_MAGIC);
    return 1;
#else
    (void)raw_x1;
    (void)screen_x1;
    (void)raw_x2;
    (void)screen_x2;
    (void)raw_y1;
    (void)screen_y1;
    (void)raw_y2;
    (void)screen_y2;
    return 0;
#endif
}
