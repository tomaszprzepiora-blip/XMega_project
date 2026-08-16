#ifndef MIKROMEDIA_XMEGA_ADAPTER_H
#define MIKROMEDIA_XMEGA_ADAPTER_H

#include <stdint.h>
#include "xmega_ui.h"

extern const ui_driver_t MIKROMEDIA_UI_DRIVER;

void mikro_board_io_init(void);
void mikro_lcd_init(void);
void mikro_backlight_set(uint8_t on);
uint8_t mikro_orientation_init(void);
uint8_t mikro_orientation_update(void);
uint8_t mikro_lcd_is_flipped(void);
void mikro_lcd_write_command(uint8_t command);
void mikro_lcd_write_data8(uint8_t data);
void mikro_lcd_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void mikro_lcd_draw_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void mikro_lcd_draw_text(uint16_t x, uint16_t y, const char *text, uint16_t color);
void mikro_esp_uart_send(const char *text);

#endif

