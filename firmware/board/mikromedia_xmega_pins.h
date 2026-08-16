#ifndef MIKROMEDIA_XMEGA_PINS_H
#define MIKROMEDIA_XMEGA_PINS_H

/*
 * Pin map for mikromedia for XMEGA, cross-checked against MikroElektronika's
 * own shipped mikroC PRO for AVR example projects (docs/xMega/mikroC PRO for
 * AVR/{TFT,Touch Panel,MMC,MP3,Serial Flash,Accel}), not just the schematic
 * PDF text extraction. TFT_RST/RS/CS/RD/WR are identical across all six
 * vendor example projects, so these bit numbers are high-confidence.
 *
 * TFT module: MI0283QT2, controller ILI9341, 320x240, resistive touch.
 * MCU: ATxmega128A1.
 */

#define MIKRO_LCD_DATA_PORT_NAME       K
#define MIKRO_LCD_DATA_NET             "T-D0..T-D7"
#define MIKRO_LCD_DATA_BITS            "PK0..PK7"

#define MIKRO_LCD_CS_PORT_NAME         J
#define MIKRO_LCD_CS_BIT               5
#define MIKRO_LCD_RS_PORT_NAME         J
#define MIKRO_LCD_RS_BIT               4
#define MIKRO_LCD_WR_PORT_NAME         J
#define MIKRO_LCD_WR_BIT               3
#define MIKRO_LCD_RD_PORT_NAME         J
#define MIKRO_LCD_RD_BIT               2
#define MIKRO_LCD_RST_PORT_NAME        J
#define MIKRO_LCD_RST_BIT              1

/*
 * BLED (backlight) is NOT consistent across vendor examples: TFT and Accel
 * use PJ0, while MMC/MP3/Serial Flash/Touch Panel use PC4 (4 vs 2). PC4 is
 * used here as the majority pick. If the backlight heartbeat blink doesn't
 * do anything visible on real hardware, try PJ0 next (see mikro_backlight_toggle
 * in mikromedia_xmega_adapter.c).
 */
#define MIKRO_LCD_BLED_PORT_NAME       C
#define MIKRO_LCD_BLED_BIT             4

#define MIKRO_TOUCH_XL_PORT_NAME       A
#define MIKRO_TOUCH_XL_BIT             2
#define MIKRO_TOUCH_YD_PORT_NAME       A
#define MIKRO_TOUCH_YD_BIT             1
#define MIKRO_TOUCH_DRIVEB_PORT_NAME   Q
#define MIKRO_TOUCH_DRIVEB_BIT         3
#define MIKRO_TOUCH_DRIVEA_PORT_NAME   Q
#define MIKRO_TOUCH_DRIVEA_BIT         2

/*
 * Convenient ESP connection on side header HDR2:
 * - HDR2 pin 47: RXD-PD2, XMEGA receives from ESP TX.
 * - HDR2 pin 48: TXD-PD3, XMEGA transmits to ESP RX.
 * - HDR2 pin 51/52: 3.3V/GND reference/power, depending on ESP current budget.
 *
 * On-board FT232RL uses RXF-MCU/TXF-MCU on PF0/PF1, so PD2/PD3 are a cleaner
 * external UART choice for ESP.
 */
#define MIKRO_ESP_UART_PORT_NAME       D
#define MIKRO_ESP_UART_RX_BIT          2
#define MIKRO_ESP_UART_TX_BIT          3
#define MIKRO_ESP_UART_HEADER_RX_PIN   47
#define MIKRO_ESP_UART_HEADER_TX_PIN   48

#endif

