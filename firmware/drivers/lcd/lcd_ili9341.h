#ifndef LCD_ILI9341_H
#define LCD_ILI9341_H

#include <stdint.h>

#define ILI9341_WIDTH  320
#define ILI9341_HEIGHT 240

typedef struct {
    void (*write_command)(uint8_t command);
    void (*write_data)(uint8_t data);
    void (*delay_ms)(uint16_t ms);
} ili9341_bus_t;

void ili9341_init(const ili9341_bus_t *bus);
void ili9341_set_rotation(const ili9341_bus_t *bus, uint8_t madctl);
void ili9341_set_window(const ili9341_bus_t *bus, uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void ili9341_fill_rect(const ili9341_bus_t *bus, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ili9341_draw_rect(const ili9341_bus_t *bus, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void ili9341_draw_text(const ili9341_bus_t *bus, uint16_t x, uint16_t y, const char *text, uint16_t color, uint16_t bg);

#endif
