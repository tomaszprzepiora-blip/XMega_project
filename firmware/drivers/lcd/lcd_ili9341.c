#include "lcd_ili9341.h"

#include <stddef.h>

#define ILI_CMD_SWRESET      0x01
#define ILI_CMD_SLPOUT       0x11
#define ILI_CMD_DISPON       0x29
#define ILI_CMD_COL_ADDR_SET 0x2A
#define ILI_CMD_PAGE_ADDR_SET 0x2B
#define ILI_CMD_MEM_WRITE    0x2C
#define ILI_CMD_MADCTL       0x36
#define ILI_CMD_PIXEL_FORMAT 0x3A

/*
 * MADCTL selects display rotation/mirroring. 0x28 gives the 320x240
 * landscape orientation matching the board's touch wiring assumption.
 * If the drawn image comes out rotated or mirrored on the real panel,
 * try 0x48, 0x88 or 0xE8 instead (the other three ILI9341 rotations).
 */
#define ILI_MADCTL_LANDSCAPE 0x28

static void delay_or_skip(const ili9341_bus_t *bus, uint16_t ms)
{
    if (bus != NULL && bus->delay_ms != NULL) {
        bus->delay_ms(ms);
    }
}

static void write_cmd(const ili9341_bus_t *bus, uint8_t command)
{
    bus->write_command(command);
}

static void write_cmd_data(const ili9341_bus_t *bus, uint8_t command, const uint8_t *data, uint8_t len)
{
    uint8_t i;

    bus->write_command(command);
    for (i = 0; i < len; i++) {
        bus->write_data(data[i]);
    }
}

static void write_pixel(const ili9341_bus_t *bus, uint16_t color)
{
    bus->write_data((uint8_t)(color >> 8));
    bus->write_data((uint8_t)color);
}

void ili9341_init(const ili9341_bus_t *bus)
{
    /*
     * 0xEF is not in the ILI9341 datasheet, but nearly every working driver
     * (Adafruit_ILI9341, TFT_eSPI, ESPHome, Teensy) sends it first. Without
     * it some ILI9341 panels/clones ignore the rest of the init sequence.
     */
    static const uint8_t unlock_ef[] = { 0x03, 0x80, 0x02 };
    static const uint8_t power_ctrl_a[] = { 0x39, 0x2C, 0x00, 0x34, 0x02 };
    static const uint8_t power_ctrl_b[] = { 0x00, 0xC1, 0x30 };
    static const uint8_t timing_ctrl_a[] = { 0x85, 0x00, 0x78 };
    static const uint8_t timing_ctrl_b[] = { 0x00, 0x00 };
    static const uint8_t power_on_seq[] = { 0x64, 0x03, 0x12, 0x81 };
    static const uint8_t pump_ratio[] = { 0x20 };
    static const uint8_t power_ctrl_1[] = { 0x23 };
    static const uint8_t power_ctrl_2[] = { 0x10 };
    static const uint8_t vcom_ctrl_1[] = { 0x3E, 0x28 };
    static const uint8_t vcom_ctrl_2[] = { 0x86 };
    static const uint8_t madctl[] = { ILI_MADCTL_LANDSCAPE };
    static const uint8_t pixel_format[] = { 0x55 }; /* 16 bpp, RGB565 */
    static const uint8_t frame_ctrl[] = { 0x00, 0x18 };
    static const uint8_t display_fn[] = { 0x08, 0x82, 0x27 };
    static const uint8_t gamma3_disable[] = { 0x00 };
    static const uint8_t gamma_curve[] = { 0x01 };
    static const uint8_t pos_gamma[] = {
        0x0F, 0x31, 0x2B, 0x0C, 0x0E, 0x08, 0x4E, 0xF1,
        0x37, 0x07, 0x10, 0x03, 0x0E, 0x09, 0x00
    };
    static const uint8_t neg_gamma[] = {
        0x00, 0x0E, 0x14, 0x03, 0x11, 0x07, 0x31, 0xC1,
        0x48, 0x08, 0x0F, 0x0C, 0x31, 0x36, 0x0F
    };

    if (bus == NULL || bus->write_command == NULL || bus->write_data == NULL) {
        return;
    }

    write_cmd_data(bus, 0xEF, unlock_ef, sizeof(unlock_ef));
    write_cmd_data(bus, 0xCB, power_ctrl_a, sizeof(power_ctrl_a));
    write_cmd_data(bus, 0xCF, power_ctrl_b, sizeof(power_ctrl_b));
    write_cmd_data(bus, 0xE8, timing_ctrl_a, sizeof(timing_ctrl_a));
    write_cmd_data(bus, 0xEA, timing_ctrl_b, sizeof(timing_ctrl_b));
    write_cmd_data(bus, 0xED, power_on_seq, sizeof(power_on_seq));
    write_cmd_data(bus, 0xF7, pump_ratio, sizeof(pump_ratio));
    write_cmd_data(bus, 0xC0, power_ctrl_1, sizeof(power_ctrl_1));
    write_cmd_data(bus, 0xC1, power_ctrl_2, sizeof(power_ctrl_2));
    write_cmd_data(bus, 0xC5, vcom_ctrl_1, sizeof(vcom_ctrl_1));
    write_cmd_data(bus, 0xC7, vcom_ctrl_2, sizeof(vcom_ctrl_2));
    write_cmd_data(bus, ILI_CMD_MADCTL, madctl, sizeof(madctl));
    write_cmd_data(bus, ILI_CMD_PIXEL_FORMAT, pixel_format, sizeof(pixel_format));
    write_cmd_data(bus, 0xB1, frame_ctrl, sizeof(frame_ctrl));
    write_cmd_data(bus, 0xB6, display_fn, sizeof(display_fn));
    write_cmd_data(bus, 0xF2, gamma3_disable, sizeof(gamma3_disable));
    write_cmd_data(bus, 0x26, gamma_curve, sizeof(gamma_curve));
    write_cmd_data(bus, 0xE0, pos_gamma, sizeof(pos_gamma));
    write_cmd_data(bus, 0xE1, neg_gamma, sizeof(neg_gamma));

    write_cmd(bus, ILI_CMD_SLPOUT);
    delay_or_skip(bus, 120);
    write_cmd(bus, ILI_CMD_DISPON);
    delay_or_skip(bus, 20);

    ili9341_fill_rect(bus, 0, 0, ILI9341_WIDTH, ILI9341_HEIGHT, 0x0000);
}

void ili9341_set_rotation(const ili9341_bus_t *bus, uint8_t madctl)
{
    if (bus == NULL || bus->write_command == NULL || bus->write_data == NULL) {
        return;
    }

    bus->write_command(ILI_CMD_MADCTL);
    bus->write_data(madctl);

    /*
     * Same settle time regardless of which direction MADCTL changed (normal
     * -> flipped or flipped -> normal), so both transitions run through one
     * identical procedure instead of one being quicker/less complete than
     * the other.
     */
    delay_or_skip(bus, 20);
}

void ili9341_set_window(const ili9341_bus_t *bus, uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    uint16_t x2;
    uint16_t y2;

    if (bus == NULL || bus->write_command == NULL || bus->write_data == NULL ||
        x >= ILI9341_WIDTH || y >= ILI9341_HEIGHT || w == 0 || h == 0) {
        return;
    }

    x2 = (uint16_t)(x + w - 1);
    y2 = (uint16_t)(y + h - 1);
    if (x2 >= ILI9341_WIDTH) {
        x2 = ILI9341_WIDTH - 1;
    }
    if (y2 >= ILI9341_HEIGHT) {
        y2 = ILI9341_HEIGHT - 1;
    }

    bus->write_command(ILI_CMD_COL_ADDR_SET);
    bus->write_data((uint8_t)(x >> 8));
    bus->write_data((uint8_t)x);
    bus->write_data((uint8_t)(x2 >> 8));
    bus->write_data((uint8_t)x2);

    bus->write_command(ILI_CMD_PAGE_ADDR_SET);
    bus->write_data((uint8_t)(y >> 8));
    bus->write_data((uint8_t)y);
    bus->write_data((uint8_t)(y2 >> 8));
    bus->write_data((uint8_t)y2);

    bus->write_command(ILI_CMD_MEM_WRITE);
}

void ili9341_fill_rect(const ili9341_bus_t *bus, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    uint32_t count;

    if (bus == NULL || bus->write_command == NULL || bus->write_data == NULL ||
        x >= ILI9341_WIDTH || y >= ILI9341_HEIGHT || w == 0 || h == 0) {
        return;
    }

    if ((uint32_t)x + w > ILI9341_WIDTH) {
        w = (uint16_t)(ILI9341_WIDTH - x);
    }
    if ((uint32_t)y + h > ILI9341_HEIGHT) {
        h = (uint16_t)(ILI9341_HEIGHT - y);
    }

    ili9341_set_window(bus, x, y, w, h);
    count = (uint32_t)w * h;
    while (count-- > 0) {
        write_pixel(bus, color);
    }
}

void ili9341_draw_rect(const ili9341_bus_t *bus, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (w == 0 || h == 0) {
        return;
    }

    ili9341_fill_rect(bus, x, y, w, 1, color);
    ili9341_fill_rect(bus, x, (uint16_t)(y + h - 1), w, 1, color);
    ili9341_fill_rect(bus, x, y, 1, h, color);
    ili9341_fill_rect(bus, (uint16_t)(x + w - 1), y, 1, h, color);
}

static uint8_t glyph_column(char ch, uint8_t col)
{
    static const uint8_t digits[10][5] = {
        {0x3E, 0x51, 0x49, 0x45, 0x3E}, {0x00, 0x42, 0x7F, 0x40, 0x00},
        {0x42, 0x61, 0x51, 0x49, 0x46}, {0x21, 0x41, 0x45, 0x4B, 0x31},
        {0x18, 0x14, 0x12, 0x7F, 0x10}, {0x27, 0x45, 0x45, 0x45, 0x39},
        {0x3C, 0x4A, 0x49, 0x49, 0x30}, {0x01, 0x71, 0x09, 0x05, 0x03},
        {0x36, 0x49, 0x49, 0x49, 0x36}, {0x06, 0x49, 0x49, 0x29, 0x1E}
    };
    static const uint8_t upper[26][5] = {
        {0x7E,0x11,0x11,0x11,0x7E}, {0x7F,0x49,0x49,0x49,0x36},
        {0x3E,0x41,0x41,0x41,0x22}, {0x7F,0x41,0x41,0x22,0x1C},
        {0x7F,0x49,0x49,0x49,0x41}, {0x7F,0x09,0x09,0x09,0x01},
        {0x3E,0x41,0x49,0x49,0x7A}, {0x7F,0x08,0x08,0x08,0x7F},
        {0x00,0x41,0x7F,0x41,0x00}, {0x20,0x40,0x41,0x3F,0x01},
        {0x7F,0x08,0x14,0x22,0x41}, {0x7F,0x40,0x40,0x40,0x40},
        {0x7F,0x02,0x0C,0x02,0x7F}, {0x7F,0x04,0x08,0x10,0x7F},
        {0x3E,0x41,0x41,0x41,0x3E}, {0x7F,0x09,0x09,0x09,0x06},
        {0x3E,0x41,0x51,0x21,0x5E}, {0x7F,0x09,0x19,0x29,0x46},
        {0x46,0x49,0x49,0x49,0x31}, {0x01,0x01,0x7F,0x01,0x01},
        {0x3F,0x40,0x40,0x40,0x3F}, {0x1F,0x20,0x40,0x20,0x1F},
        {0x7F,0x20,0x18,0x20,0x7F}, {0x63,0x14,0x08,0x14,0x63},
        {0x07,0x08,0x70,0x08,0x07}, {0x61,0x51,0x49,0x45,0x43}
    };

    if (col >= 5) {
        return 0;
    }
    if (ch >= '0' && ch <= '9') {
        return digits[ch - '0'][col];
    }
    if (ch >= 'a' && ch <= 'z') {
        ch = (char)(ch - 'a' + 'A');
    }
    if (ch >= 'A' && ch <= 'Z') {
        return upper[ch - 'A'][col];
    }
    if (ch == '.') {
        return col == 2 ? 0x40 : 0x00;
    }
    if (ch == ':') {
        return col == 2 ? 0x14 : 0x00;
    }
    if (ch == '-') {
        return col == 2 ? 0x08 : 0x00;
    }
    if (ch == ' ') {
        return 0x00;
    }
    return 0x49;
}

static void draw_char(const ili9341_bus_t *bus, uint16_t x, uint16_t y, char ch, uint16_t color, uint16_t bg)
{
    uint8_t col;
    uint8_t row;
    uint8_t bits;

    for (col = 0; col < 6; col++) {
        bits = glyph_column(ch, col);
        for (row = 0; row < 8; row++) {
            ili9341_fill_rect(bus, (uint16_t)(x + col), (uint16_t)(y + row), 1, 1,
                              (bits & (1u << row)) ? color : bg);
        }
    }
}

void ili9341_draw_text(const ili9341_bus_t *bus, uint16_t x, uint16_t y, const char *text, uint16_t color, uint16_t bg)
{
    while (text != NULL && *text != '\0' && x + 6 < ILI9341_WIDTH) {
        draw_char(bus, x, y, *text, color, bg);
        x = (uint16_t)(x + 6);
        text++;
    }
}
