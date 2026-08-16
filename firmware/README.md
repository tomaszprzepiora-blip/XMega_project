# Firmware notes

Ten katalog zawiera pierwsza warstwe logiki UI dla `ATxmega128A1`.

Pliki:

- `app/main.c` - minimalny firmware startowy rysujacy panel na LCD.
- `Makefile` - budowanie i wgrywanie przez AVRISP mkII/PDI.
- `ui/xmega_ui_layout.h` - geometria ekranu i 9 kontrolek dotykowych.
- `ui/xmega_ui.h` / `ui/xmega_ui.c` - stan UI, hit-test, rysowanie przez hooki, wysylanie komend UART do ESP i parsowanie odpowiedzi.
- `drivers/lcd/lcd_ili9341.h` / `drivers/lcd/lcd_ili9341.c` - sterownik LCD ILI9341 dla modulu MI0283QT2.
- `board/mikromedia_xmega_adapter.h` / `board/mikromedia_xmega_adapter.c` - adapter pinow mikromedia for XMEGA do sterownika UI/LCD.
- `board/mikromedia_xmega_pins.h` - opis polaczen LCD, podswietlenia, touch i UART ESP.
- `protocol/esp_protocol.h` / `protocol/esp_protocol.c` - parser odpowiedzi ESP i generator komend tekstowych.
- `touch/touch_resistive.h` / `touch/touch_resistive.c` - pierwszy odczyt rezystancyjnego dotyku z kalibracja w jednym miejscu.
- `transport/uart_esp.h` / `transport/uart_esp.c` - UART na `PD2/PD3` do modulu ESP.

Aktualny firmware uzywa gotowego adaptera `MIKROMEDIA_UI_DRIVER`, ktory laczy UI ze sterownikiem LCD, UART-em i pinami plytki:

```c
mikro_board_io_init();
uart_esp_init(115200);
mikro_lcd_init();

ui_init(&ui);
ui_draw_full(&MIKROMEDIA_UI_DRIVER, &ui);
```

Glowna petla odbiera linie tekstowe z ESP, aktualizuje statusy/temperatury i obsluguje dotyk:

```c
if (uart_esp_read_line(esp_line, sizeof(esp_line))) {
    ui_apply_esp_line(&MIKROMEDIA_UI_DRIVER, &ui, esp_line);
}

if (touch_resistive_poll(&touch)) {
    ui_touch_down(&MIKROMEDIA_UI_DRIVER, &ui, touch.x, touch.y);
}
```

Kolory sa w formacie RGB565.

Build i flash:

```sh
PATH=/usr/local/opt/avr-gcc@14/bin:/usr/local/bin:$PATH make -C firmware
PATH=/usr/local/opt/avr-gcc@14/bin:/usr/local/bin:$PATH make -C firmware flash
```

Ostatni sprawdzony wynik:

- `build/xmega_domoticz_panel.hex`
- flash: `7516 bytes`
- RAM: `604 bytes`
- `avrdude`: zapis i weryfikacja przez AVRISP mkII/PDI zakonczone sukcesem.

Testy hostowe:

```sh
sh tests/run_host_tests.sh
```
