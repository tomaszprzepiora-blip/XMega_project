#include "mikromedia_xmega_adapter.h"
#include "adxl345.h"
#include "lcd_ili9341.h"
#include "mikromedia_xmega_pins.h"
#include "uart_esp.h"

/*
 * MADCTL values for the two supported landscape orientations. 0x28 matches
 * the default set in ili9341_init(); 0xE8 is the same landscape mode rotated
 * 180 degrees (MY|MX|MV|BGR).
 */
#define ORIENTATION_MADCTL_NORMAL  0x28
#define ORIENTATION_MADCTL_FLIPPED 0xE8

/*
 * Which ADXL345 axis/sign means "flipped" depends on how the accelerometer
 * die is physically oriented on the board relative to the screen, which is
 * not documented in the schematic. This starts with the Y axis (as read in
 * the vendor's own docs/xMega Accel demo) and a +/-150 LSB deadband (full
 * resolution, 4 mg/LSB, so ~600 mg) to avoid flipping on a nearly-flat panel.
 * If real hardware flips backwards or doesn't flip at all, swap the axis
 * read below (sample.y -> sample.x) or invert the comparison signs.
 */
#define ORIENTATION_FLIP_THRESHOLD 150

/*
 * Require the SAME candidate orientation to be read this many consecutive
 * times (mikro_orientation_update runs every ~250 ms) before actually
 * committing to it and redrawing. Without this, a reading that sits right
 * at the threshold - which happens more at one physical angle than the
 * other, since the accelerometer axis isn't perfectly aligned with the
 * panel - can cross back and forth on small hand jitter and cause repeated
 * full-screen redraws (visible as flicker) at that orientation only.
 */
#define ORIENTATION_CONFIRM_TICKS 3

static uint8_t lcd_flipped;
static uint8_t orientation_pending;
static uint8_t orientation_confirm_count;

/*
 * Hardware adapter skeleton for mikromedia for XMEGA.
 *
 * This file intentionally keeps the low-level LCD/touch code as small hooks.
 * The next step is to bind these functions either to a MikroElektronika TFT
 * library or to a local MI0283QT2/ILI9341-compatible driver.
 */

#if defined(__AVR_XMEGA__)
#include <avr/io.h>
#include <util/delay.h>

#define BIT_MASK(bit) (1u << (bit))

static void board_clock_init(void)
{
    /*
     * ATxmega startuje z bezpiecznym zegarem po resecie. Dla UART 115200 i
     * sensownych opoznien ustawiamy wewnetrzny RC32M jako zegar systemowy.
     */
    OSC.CTRL |= OSC_RC32MEN_bm;
    while ((OSC.STATUS & OSC_RC32MRDY_bm) == 0) {
    }

    CCP = CCP_IOREG_gc;
    CLK.CTRL = CLK_SCLKSEL_RC32M_gc;
}

static void lcd_write_bus(uint8_t value)
{
    PORTK.OUT = value;
}

static void lcd_wr_pulse(void)
{
    PORTJ.OUTCLR = BIT_MASK(MIKRO_LCD_WR_BIT);
    PORTJ.OUTSET = BIT_MASK(MIKRO_LCD_WR_BIT);
}

static void lcd_select_command(void)
{
    PORTJ.OUTCLR = BIT_MASK(MIKRO_LCD_RS_BIT);
    PORTJ.OUTCLR = BIT_MASK(MIKRO_LCD_CS_BIT);
}

static void lcd_select_data(void)
{
    PORTJ.OUTSET = BIT_MASK(MIKRO_LCD_RS_BIT);
    PORTJ.OUTCLR = BIT_MASK(MIKRO_LCD_CS_BIT);
}

static void lcd_unselect(void)
{
    PORTJ.OUTSET = BIT_MASK(MIKRO_LCD_CS_BIT);
}

void mikro_lcd_write_command(uint8_t command)
{
    lcd_select_command();
    lcd_write_bus(command);
    lcd_wr_pulse();
    lcd_unselect();
}

void mikro_lcd_write_data8(uint8_t data)
{
    lcd_select_data();
    lcd_write_bus(data);
    lcd_wr_pulse();
    lcd_unselect();
}

static void mikro_delay_ms(uint16_t ms)
{
    while (ms-- > 0) {
        _delay_ms(1);
    }
}

void mikro_board_io_init(void)
{
    board_clock_init();

    PORTK.DIR = 0xFF;
    PORTJ.DIRSET = BIT_MASK(MIKRO_LCD_CS_BIT) |
                   BIT_MASK(MIKRO_LCD_RS_BIT) |
                   BIT_MASK(MIKRO_LCD_WR_BIT) |
                   BIT_MASK(MIKRO_LCD_RD_BIT) |
                   BIT_MASK(MIKRO_LCD_RST_BIT);
    PORTC.DIRSET = BIT_MASK(MIKRO_LCD_BLED_BIT);

    PORTJ.OUTSET = BIT_MASK(MIKRO_LCD_CS_BIT) |
                   BIT_MASK(MIKRO_LCD_WR_BIT) |
                   BIT_MASK(MIKRO_LCD_RD_BIT);
    /*
     * LCD-RST trzymamy nisko (aktywny reset) az do wlasciwej inicjalizacji
     * panelu w mikro_lcd_reset(). Bez sprzetowego impulsu resetu ILI9341
     * moze zostac w stanie sprzed poprzedniego programowania i nie przyjac
     * poprawnie kolejnej sekwencji init/GRAM clear.
     */
    PORTJ.OUTCLR = BIT_MASK(MIKRO_LCD_RST_BIT);
    PORTC.OUTSET = BIT_MASK(MIKRO_LCD_BLED_BIT);
}

static void mikro_lcd_reset(void)
{
    PORTJ.OUTCLR = BIT_MASK(MIKRO_LCD_RST_BIT);
    mikro_delay_ms(20);
    PORTJ.OUTSET = BIT_MASK(MIKRO_LCD_RST_BIT);
    mikro_delay_ms(120);
}

void mikro_backlight_set(uint8_t on)
{
    if (on) {
        PORTC.OUTSET = BIT_MASK(MIKRO_LCD_BLED_BIT);
    } else {
        PORTC.OUTCLR = BIT_MASK(MIKRO_LCD_BLED_BIT);
    }
}

#else

void mikro_board_io_init(void)
{
}

void mikro_lcd_write_command(uint8_t command)
{
    (void)command;
}

void mikro_lcd_write_data8(uint8_t data)
{
    (void)data;
}

static void mikro_delay_ms(uint16_t ms)
{
    (void)ms;
}

static void mikro_lcd_reset(void)
{
}

void mikro_backlight_set(uint8_t on)
{
    (void)on;
}

#endif

static const ili9341_bus_t MIKRO_LCD_BUS = {
    mikro_lcd_write_command,
    mikro_lcd_write_data8,
    mikro_delay_ms
};

void mikro_lcd_init(void)
{
    mikro_lcd_reset();
    ili9341_init(&MIKRO_LCD_BUS);
}

uint8_t mikro_orientation_init(void)
{
    lcd_flipped = 0;
    orientation_pending = 0;
    orientation_confirm_count = 0;
    return adxl345_init();
}

uint8_t mikro_orientation_update(void)
{
    accel_sample_t sample;
    uint8_t want_flipped;

    if (!adxl345_read(&sample)) {
        return 0;
    }

    if (sample.y > ORIENTATION_FLIP_THRESHOLD) {
        want_flipped = 1;
    } else if (sample.y < -ORIENTATION_FLIP_THRESHOLD) {
        want_flipped = 0;
    } else {
        orientation_confirm_count = 0;
        return 0;
    }

    if (want_flipped == lcd_flipped) {
        orientation_confirm_count = 0;
        return 0;
    }

    if (want_flipped == orientation_pending) {
        orientation_confirm_count++;
    } else {
        orientation_pending = want_flipped;
        orientation_confirm_count = 1;
    }

    if (orientation_confirm_count < ORIENTATION_CONFIRM_TICKS) {
        return 0;
    }

    orientation_confirm_count = 0;
    lcd_flipped = want_flipped;
    ili9341_set_rotation(&MIKRO_LCD_BUS,
                          lcd_flipped ? ORIENTATION_MADCTL_FLIPPED : ORIENTATION_MADCTL_NORMAL);
    return 1;
}

uint8_t mikro_lcd_is_flipped(void)
{
    return lcd_flipped;
}

void mikro_lcd_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    ili9341_fill_rect(&MIKRO_LCD_BUS, x, y, w, h, color);
}

void mikro_lcd_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    ili9341_draw_rect(&MIKRO_LCD_BUS, x, y, w, h, color);
}

void mikro_lcd_draw_text(uint16_t x, uint16_t y, const char *text, uint16_t color)
{
    ili9341_draw_text(&MIKRO_LCD_BUS, x, y, text, color, 0x1082);
}

void mikro_esp_uart_send(const char *text)
{
    uart_esp_send(text);
}

const ui_driver_t MIKROMEDIA_UI_DRIVER = {
    mikro_lcd_fill_rect,
    mikro_lcd_draw_rect,
    mikro_lcd_draw_text,
    mikro_esp_uart_send
};
