#include "mikromedia_xmega_adapter.h"
#include "touch_resistive.h"
#include "uart_debug.h"
#include "uart_esp.h"
#include "xmega_menu.h"
#include "xmega_ui.h"

#include <stdio.h>

#if defined(__AVR_XMEGA__)
#include <avr/io.h>
#include <util/delay.h>
#endif

/* Petla glowna spi co 10 ms, wiec 10 s bezczynnosci to 1000 tikow. */
#define BACKLIGHT_TIMEOUT_TICKS 1000U

/*
 * Przytrzymanie dotyku w belce statusu przez ~8 wykryc pod rzad otwiera
 * menu. Dotyk powtarza sie z gory co ~180 ms (touch_resistive.c), wiec to
 * ok. 1.4 s.
 *
 * Ten gest sprawdza SUROWY raw_y (nie skalibrowane touch.y): "belka statusu"
 * musi byc rozpoznawalna nawet gdy kalibracja jest zla/nieudana - inaczej
 * nie da sie ponownie wejsc do menu, by ja poprawic. Ta os jest odwrocona
 * (patrz touch_resistive.c) - wieksze raw_y = blizej gory ekranu. Pierwszy
 * rzad przyciskow (ekranowe y=100) odpowiada raw_y ok. 2030; belka statusu
 * (y<96) siega powyzej ok. 2060+, wiec prog ustawiono z zapasem na 2200.
 */
#define MENU_HOLD_TICKS 8U
#define MENU_GESTURE_RAW_Y_MIN 2200U

int main(void)
{
    ui_state_t ui;
    touch_point_t touch;
    char esp_line[48];
    uint8_t menu_hold_count = 0;
    uint8_t menu_gap_ticks = 0;
    uint16_t backlight_idle_ticks = 0;
    uint8_t backlight_on = 1;

    /*
     * Kolejnosc startu jest celowo prosta:
     * 1. ustawiamy piny plytki,
     * 2. inicjalizujemy kontroler LCD,
     * 3. budujemy stan UI,
     * 4. rysujemy caly ekran.
     *
     * Petla glowna nizej jest event-loopem: kazdy sterownik dostarcza male
     * zdarzenia, a UI tylko aktualizuje stan i rysuje zmienione fragmenty.
     *
     * Automatyczny obrot ekranu o 180 (akcelerometr + MADCTL) jest na razie
     * wylaczony - mikro_orientation_init()/mikro_orientation_update() z
     * mikromedia_xmega_adapter.c nie sa tu wolane. Kod dalej istnieje, gdyby
     * mial wrocic, ale dodawal zmienna, ktora trzeba bylo brac pod uwage
     * przy kalibracji dotyku.
     */
    mikro_board_io_init();
    touch_resistive_init();
    uart_esp_init(115200);
    uart_debug_init(9600);
    mikro_lcd_init();

    uart_debug_send("mikromedia XMEGA panel start\r\n");

    ui_init(&ui);
    ui_draw_full(&MIKROMEDIA_UI_DRIVER, &ui);

    /* Pierwsze uruchomienie (brak zapisanej kalibracji w EEPROM): od razu
     * poprosic o kalibracje dotyku, zanim panel bedzie normalnie uzywany. */
    if (!touch_resistive_load_calibration()) {
        menu_run(&MIKROMEDIA_UI_DRIVER);
        ui_draw_full(&MIKROMEDIA_UI_DRIVER, &ui);
    }

    for (;;) {
        if (uart_esp_read_line(esp_line, sizeof(esp_line))) {
            ui_apply_esp_line(&MIKROMEDIA_UI_DRIVER, &ui, esp_line);
        }

        if (touch_resistive_poll(&touch)) {
            backlight_idle_ticks = 0;
            if (!backlight_on) {
                backlight_on = 1;
                mikro_backlight_set(1);
            }

            {
                uint16_t raw_x;
                uint16_t raw_y;

                touch_resistive_last_raw(&raw_x, &raw_y);
                if (raw_y > MENU_GESTURE_RAW_Y_MIN) {
                    menu_gap_ticks = 0;
                    if (++menu_hold_count >= MENU_HOLD_TICKS) {
                        menu_hold_count = 0;
                        menu_run(&MIKROMEDIA_UI_DRIVER);
                        ui_draw_full(&MIKROMEDIA_UI_DRIVER, &ui);
                    }
                } else {
                    char line[48];
                    int8_t hit;

                    menu_hold_count = 0;
                    hit = ui_hit_test(touch.x, touch.y);
                    (void)snprintf(line, sizeof(line), "touch: x=%u y=%u hit=%d\r\n", touch.x, touch.y, hit);
                    uart_debug_send(line);
                    ui_touch_down(&MIKROMEDIA_UI_DRIVER, &ui, touch.x, touch.y);

                    /*
                     * Salon/Kuchnia/Korytarz/Biurko/Noc keep a persistent
                     * on/off color (ui_touch_down sets UI_CONTROL_PENDING
                     * for those). All Off/Temp 1/Temp 2/Sync are one-shot
                     * actions with no such state, so without this they give
                     * no visible confirmation that the touch registered -
                     * flash them briefly instead.
                     */
                    if (hit >= 0 && UI_CONTROLS[hit].id > 5U) {
                        const ui_control_t *c = &UI_CONTROLS[hit];

                        MIKROMEDIA_UI_DRIVER.fill_rect(c->x, c->y, c->w, c->h, 0x8400);
                        MIKROMEDIA_UI_DRIVER.draw_rect(c->x, c->y, c->w, c->h, 0x4208);
                        MIKROMEDIA_UI_DRIVER.draw_text((uint16_t)(c->x + 8), (uint16_t)(c->y + 14),
                                                        c->label, 0xFFFF);
#if defined(__AVR_XMEGA__)
                        _delay_ms(150);
#endif
                        ui_draw_control(&MIKROMEDIA_UI_DRIVER, &ui, (uint8_t)hit);
                    }
                }
            }
        }

#if defined(__AVR_XMEGA__)
        /*
         * touch_resistive_poll() only fires roughly every ~180 ms while a
         * finger stays down (its own internal repeat guard), so a held
         * status-bar touch sees many "no touch this tick" iterations in
         * between - do not treat those as a release. Only reset the hold
         * count after a real gap (no detection for a while).
         */
        if (menu_hold_count > 0 && ++menu_gap_ticks > 40U) {
            menu_hold_count = 0;
            menu_gap_ticks = 0;
        }

        _delay_ms(10);
        if (backlight_on && ++backlight_idle_ticks >= BACKLIGHT_TIMEOUT_TICKS) {
            backlight_on = 0;
            mikro_backlight_set(0);
        }
#endif
    }
}
