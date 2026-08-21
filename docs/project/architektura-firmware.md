# Architektura firmware

Firmware jest podzielony tak, zeby kazdy plik mial jedna odpowiedzialnosc.

## `firmware/app`

Punkt startowy programu.

- `main.c` inicjalizuje plytke, UART (ESP + diagnostyczny), LCD i UI.
- Petla glowna obsluguje: linie z ESP (w tym maszyne stanow RCV/retry/wynik dla pojedynczych komend, patrz `firmware/protocol`), dotyk (przyciski albo - gdy dotyk jest w belce statusu przez ~1.4s - otwarcie menu), auto-wygaszanie podswietlenia po 10s bezczynnosci.
- Automatyczny obrot ekranu o 180 (akcelerometr) jest na razie wylaczony (nie wolany z `main.c`), zeby nie dodawac zmiennej podczas kalibracji dotyku - kod nadal istnieje w `board`/`drivers/lcd`.

## `firmware/board`

Warstwa konkretnej plytki `mikromedia for XMEGA`.

- `mikromedia_xmega_pins.h` opisuje piny z dokumentacji.
- `mikromedia_xmega_adapter.c` laczy UI z fizycznym LCD i UART.

Ta warstwa jest jedynym miejscem, ktore powinno bezposrednio znac porty `PORTK`, `PORTJ`, `PORTC`, `PORTD`.

## `firmware/drivers/lcd`

Sterownik kontrolera LCD.

- `lcd_ili9341.c` zna komendy ILI9341, okna rysowania i format pikseli.
- Nie zna Domoticza, ESP ani logiki przyciskow.

## `firmware/drivers/accel`

Sterownik akcelerometru `ADXL345` (I2C, wbudowany na plytce).

- `adxl345.c` zna rejestry ADXL345 i odczyt X/Y/Z. Korzysta z `transport/i2c_twid.c`.
- Nie zna LCD, UI ani decyzji o obrocie ekranu - to logika adaptera.

## `firmware/touch`

Odczyt panelu dotykowego rezystancyjnego.

- `touch_resistive.c` steruje `DRIVEA`/`DRIVEB` (`PQ2`/`PQ3`), czyta ADC (`PA1`=`YD`, `PA2`=`XL`), i zamienia surowy odczyt na wspolrzedne ekranowe.
- Wazne: elektryczne osie "X"/"Y" folii dotykowej sa obrocone o 90 stopni wzgledem obrazu LCD i dzialaja odwrotnie (wiekszy raw = mniejsza wspolrzedna ekranowa) - `touch_resistive_poll()` zamienia je na wyjsciu, a `map_axis()` liczy ze znakiem, zeby to obsluzyc. Patrz komentarz na szczycie pliku.
- Kalibracja (`touch_resistive_calibrate()`/`touch_resistive_load_calibration()`) jest zmienna w RAM, zapisywana w EEPROM (`avr/eeprom.h`), nie stalym `#define` - kazda plytka/panel moze miec inny zakres ADC.
- Nie wie nic o ekranie/przyciskach - tylko dostarcza `touch_point_t` i surowe wartosci (`touch_resistive_last_raw()`) do warstwy wyzej.

## `firmware/ui`

Widok, lokalny stan panelu i menu urzadzenia.

- `xmega_ui_layout.h` trzyma geometrie 9 kontrolek.
- `xmega_ui.c` rysuje ekran, sprawdza dotyk po wspolrzednych (po kalibracji z `touch_resistive.c`) i aktualizuje stan. Przyciski 1-5 (Lozko Aga..Salon Spr.) przelaczaja sie (wlacz/wylacz) miedzy dotykami; 6-9 (All Off/Temp 1/Temp 2/Sync) to akcje jednorazowe bez trwalego stanu.
- `domoticz_map.h` trzyma mapowanie kontrolek UI na idx urzadzen Domoticza (`UI_SWITCH_IDX`, `UI_TEMP_IDX`) - XMEGA zna idx wprost, ESP nie musi znac zadnego mapowania przycisk->idx.
- `xmega_menu.c` to ekran menu na urzadzeniu (obecnie: kalibracja dotyku po 9 przyciskach), rozwijalny o kolejne pozycje. Wejscie: przytrzymanie dotyku w belce statusu (`main.c`); nawigacja w menu jest niezalezna od kalibracji ekranowej (dziala nawet gdy kalibracja jest zla/brakujaca), bo to jest miejsce, w ktorym sie ja naprawia.

UI komunikuje sie ze swiatem przez `ui_driver_t`, czyli przez funkcje: `fill_rect`, `draw_rect`, `draw_text`, `uart_send`.

## `firmware/protocol`

Format tekstowych wiadomosci XMEGA <-> ESP. XMEGA zna idx urzadzen Domoticza
wprost (`firmware/protocol/domoticz_map.h`) i wysyla je razem z docelowa
wartoscia - ESP nie zna zadnej logiki "przyciskow", tylko tlumaczy
`idx + wartosc` na wywolanie Domoticz HTTP API.

Przyklady:

```text
idx:32:1      XMEGA -> ESP: ustaw przelacznik idx=32 na On
idx:32:0      XMEGA -> ESP: ustaw przelacznik idx=32 na Off
idx:40:?      XMEGA -> ESP: zapytaj o wartosc idx=40
RCV,32        ESP -> XMEGA: natychmiastowe potwierdzenie odbioru linii (przed HTTP)
idx:40:216    ESP -> XMEGA: wynik/potwierdzenie wykonania (216 = 21.6 C dla czujnika, 0/1 dla przelacznika)
ERR,IDX,40    ESP -> XMEGA: blad HTTP/Domoticz dla idx=40 (wykonano z bledem)
```

Dwie fazy odpowiedzi (`RCV,` zaraz po odebraniu, potem `idx:.../ERR,IDX,` po
zakonczeniu HTTP do Domoticza) pozwalaja XMEGA odroznic "ESP nie zyje na
UART" od "ESP zyje, ale Domoticz odpowiada wolno" - patrz `ESP_PHASE_WAIT_RCV`/
`ESP_PHASE_WAIT_RESULT` w `firmware/app/main.c`. Dotyczy to tylko komend
pojedynczych (przelaczniki, Temp 1/2, `ui.pending_retryable`) - jesli po
`ESP_RCV_TIMEOUT_TICKS` nie przyjdzie `RCV,`, XMEGA ponawia wyslanie tej samej
linii (do `ESP_MAX_SEND_ATTEMPTS` razy) zanim uzna link UART<->ESP za martwy.
All Off/Sync wysylaja kilka linii na raz i nie sa ponawiane.

Jesli protokol sie zmieni, najpierw edytowac ten modul, nie UI ani sterownik LCD.

## `firmware/transport`

Niskopoziomowa transmisja.

- `uart_esp.c` obsluguje UART `PD2/PD3`.
- `i2c_twid.c` obsluguje magistrale I2C `TWID` (`PD0/PD1`) do akcelerometru.
- `uart_debug.c` obsluguje `USARTF0` (`PF2`=`RXD0`, `PF3`=`TXD0`) do wbudowanego USB-UART (`FT232RL`) - konsola diagnostyczna widoczna na PC jako `/dev/cu.usbserial-XXXXXXXX`, 9600 baud.
- Nie interpretuje tresci wiadomosci.

## `esp`

Oddzielny program dla ESP8266/ESP32.

- Odbiera tekst po UART.
- Wysyla HTTP JSON API do Domoticza.
- Odsyla tekstowe odpowiedzi do XMEGA.

