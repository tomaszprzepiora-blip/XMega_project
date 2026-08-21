# XMega / mikromedia for XMEGA

Workspace dla plytki **mikromedia for XMEGA** firmy MikroElektronika.

## Zidentyfikowany sprzet

- Plytka: `mikromedia for XMEGA`, dokumentacja `v111b`, najpewniej produkt MIKROE-755.
- MCU: `ATxmega128A1`.
- Programowanie: bootloader UART albo zewnetrzny programator PDI przez zlacze `CN5`.
- Programator podlaczony do komputera: `AVRISP mkII`, USB VID:PID `03eb:2104`, serial `000200212345`.

## Pobrane dokumenty

- `docs/mikromedia-xmega-manual-v111b.pdf` - instrukcja uzytkownika, 40 stron.
- `docs/mikromedia-xmega-schematic-v111b.pdf` - schemat plytki, 3 strony.
- `docs/mikromedia-xmega-pinout-v111b.pdf` - pinout i wymiary, 2 strony.
- `docs/ATxmega64A1-ATxmega128A1-datasheet.pdf` - datasheet MCU.
- `docs/AVRISP-mkII-user-guide.pdf` - instrukcja AVRISP mkII.

## Istotne uklady na plytce

- `ATxmega128A1` - glowny mikrokontroler AVR XMEGA.
- `FT232RL` - USB-UART dla bootloadera/komunikacji z PC.
- `VS1053` - kodek/dekoder audio MP3.
- `M25P80` - zewnetrzna pamiec SPI Flash 8 Mbit.
- `MCP73832` - ladowarka Li-Polymer.
- TFT 320x240 z rezystancyjnym panelem dotykowym.
- microSD, gniazdo sluchawkowe 3.5 mm, zlacze Mini-B USB, boczne pady 2x26.

## PDI / CN5

Manual Mikroe opisuje programowanie zewnetrzne tak:

- programator: `AVRISP mkII`, `AVR JTAGICE mkII` lub inny z PDI;
- zlacze na plytce: pady `PDI (CN5)`;
- trzeba miec wlutowany header `2x3` na `CN5`;
- jesli bootloader zostanie skasowany, mozna go wgrac ponownie przez PDI.

Schemat `CN5` pokazuje sygnaly:

- `PDI` - linia danych PDI do pinu `PDI` w `ATxmega128A1`;
- `RESET#` - linia reset / PDI clock do pinu `RESET/PDI`;
- `VCC-3.3` - odniesienie napiecia targetu;
- `GND`.

Uwaga z instrukcji AVRISP mkII: pin `VCC` nie zasila targetu w oryginalnym AVRISP mkII, tylko mierzy napiecie targetu. Plytka musi byc zasilona osobno, np. przez USB/baterie, a `VCC-3.3` musi byc podlaczone do programatora jako reference.

## Lokalny test

Wykonane sprawdzenia:

```sh
system_profiler SPUSBDataType
ioreg -p IOUSB -l -w 0
avrdude -v
avrdude -c '?' | rg -i 'avr.*mk|pdi|xmega'
avrdude -p '?' | rg -i 'xmega128a1|x128a1|atxmega128a1'
avrdude -c avrispmkII -p x128a1 -P usb -v
```

Wynik pierwszego sprawdzenia:

- `avrdude` jest zainstalowany: wersja `8.2`.
- `avrdude` zna programator `avrispmkII` / `avrisp2` z obsluga `TPI, ISP, PDI`.
- `avrdude` zna target `x128a1 = ATxmega128A1 (SPM, PDI, XMEGAJTAG)`.
- macOS widzi USB `AVRISP mkII`.
- Odczyt przez `avrdude` nie doszedl jeszcze do targetu, bo dostep do USB zostal zablokowany:

```text
Error: found but could not access USB device usb (03eb:2104)
Error: unable to open port usb for programmer avrispmkII
```

Wynik ponownego sprawdzenia po poprawieniu dostepu/podlaczenia:

```text
Using programmer      : avrispmkII
Programmer model      : AVRISP mkII
Serial number         : 000200212345
FW version controller : 1.41
Vtarget               : 3.3 V
Silicon revision      : 0.7
Device signature      : 1E 97 4C (ATxmega128A1, ATxmega128A1U)
```

Status: komunikacja USB z programatorem i PDI z ukladem dziala. Plytka odpowiada jako `ATxmega128A1`.

## Aktualny status firmware

Firmware panelu Domoticz zostal zbudowany i wgrany przez AVRISP mkII/PDI.

Wykonane:

```sh
sh tests/run_host_tests.sh
PATH=/usr/local/opt/avr-gcc@14/bin:/usr/local/bin:$PATH make -C firmware
PATH=/usr/local/opt/avr-gcc@14/bin:/usr/local/bin:$PATH make -C firmware flash
avrdude -c avrispmkII -p x128a1 -P usb -v
```

Wynik:

- testy hostowe: `xmega_ui tests OK`, `esp_protocol tests OK`;
- build AVR: `build/xmega_domoticz_panel.hex`;
- rozmiar: `7070 bytes` flash, `539 bytes` RAM;
- flash przez PDI: zapis i weryfikacja `7070 bytes of flash verified`;
- odczyt po flashu: `Device signature = 1E 97 4C`, `Vtarget = 3.3 V`.

Kolejna sesja programowania (rebuild + reflash tego samego firmware):

- rozmiar: `7470 bytes` flash, `540 bytes` RAM (nieznacznie wiecej niz poprzednio, prawdopodobnie inna wersja `avr-gcc` a nie zmiana kodu);
- odczyt sygnatury przed zapisem: `Device signature = 1E 97 4C`, `Vtarget = 3.3 V`;
- flash przez PDI: zapis i weryfikacja `7470 bytes of flash verified`;
- odczyt po flashu: `Device signature = 1E 97 4C`, `Vtarget = 3.3 V` - plytka odpowiada poprawnie.

Po tym flashu na sprzecie ekran zostal ze starego, poprzedniego stanu (nie zainicjalizowal sie od nowa). Przyczyna: `firmware/board/mikromedia_xmega_adapter.c` ustawial pin `LCD-RST` (`PJ2`) tylko raz na stan wysoki i nigdy go nie impulsowal - kontroler HX8347-D nie dostawal sprzetowego resetu przed sekwencja inicjalizacyjna.

Poprawka: `mikro_board_io_init()` trzyma teraz `LCD-RST` nisko (aktywny reset) od startu, a nowa funkcja `mikro_lcd_reset()` (wolana z `mikro_lcd_init()` przed `hx8347d_init`) robi wlasciwy impuls - nisko 20 ms, potem wysoko i 120 ms odczekania - zanim popłyną komendy do LCD.

Rebuild + reflash po poprawce:

- testy hostowe: `xmega_ui tests OK`, `esp_protocol tests OK`;
- rozmiar: `7514 bytes` flash, `540 bytes` RAM;
- flash przez PDI: zapis i weryfikacja `7514 bytes of flash verified`;
- na sprzecie ekran dalej sie nie inicjalizowal - okazalo sie, ze zalozenie o kontrolerze bylo bledne.

Prawdziwy kontroler na module TFT to `ILI9341` (potwierdzone przez uzytkownika bezposrednio z oznaczenia na plytce), nie `HX8347-D` jak zakladalo README/dokumentacja architektury. Caly sterownik LCD zostal przepisany pod ILI9341:

- nowy plik `firmware/drivers/lcd/lcd_ili9341.c` / `.h` zastapil usuniety `lcd_hx8347d.c` / `.h`;
- rejestry, format pikseli (bezposrednio RGB565 16bpp zamiast konwersji do RGB666) i sekwencja inicjalizacyjna sa teraz zgodne z ILI9341 (na bazie sprawdzonych, powszechnie uzywanych sekwencji z bibliotek Adafruit_ILI9341 / TFT_eSPI / ESPHome);
- init zawiera niedokumentowana w datasheet ILI9341, ale powszechnie wymagana komende `0xEF` (`0x03, 0x80, 0x02`) na samym poczatku - bez niej wiele klonow ILI9341 ignoruje reszte sekwencji init;
- orientacja ekranu (`MADCTL`) ustawiona wstepnie na `0x28` (landscape) - do doprecyzowania po sprawdzeniu obrazu na sprzecie (alternatywy: `0x48`, `0x88`, `0xE8`).

Rebuild + reflash po zmianie sterownika na ILI9341:

- testy hostowe: `xmega_ui tests OK`, `esp_protocol tests OK`;
- rozmiar: `7516 bytes` flash, `604 bytes` RAM;
- flash przez PDI: zapis i weryfikacja `7516 bytes of flash verified`;
- do zweryfikowania na sprzecie: czy ekran teraz poprawnie sie czysci/inicjalizuje i czy orientacja `MADCTL` jest poprawna.

Ekran dalej "milczal" (bez zadnej widocznej reakcji), a plytka nie ma wolnej diody LED pod GPIO - jedyne diody to ladowanie baterii i RX/TX na `FT232RL` (obie sterowane sprzetowo, poza kontrola firmware). Dlatego dodano "heartbeat" na podswietleniu LCD:

- nowa funkcja `mikro_backlight_toggle()` w `firmware/board/mikromedia_xmega_adapter.c` przelacza pin `LCD-BLED` (`PC3`);
- w glownej petli `firmware/app/main.c` podswietlenie miga co ok. 250 ms;
- cel: potwierdzic, ze firmware w ogole dziala (petla glowna zyje), niezaleznie od tego czy komunikacja z ILI9341 jest poprawna.

Rebuild + reflash z heartbeatem na podswietleniu:

- testy hostowe: `xmega_ui tests OK`, `esp_protocol tests OK`;
- rozmiar: `7540 bytes` flash, `604 bytes` RAM;
- flash przez PDI: zapis i weryfikacja `7540 bytes of flash verified`;
- do zweryfikowania na sprzecie: czy podswietlenie LCD faktycznie miga (potwierdzi, ze MCU zyje) oraz czy sam obraz na ekranie w koncu sie pojawia.

### Znaleziony prawdziwy root cause: bledne mapowanie pinow LCD

Uzytkownik dostarczyl oryginalny pakiet MikroElektroniki dla tej plytki (`docs/xMega/`) - gotowe projekty mikroC PRO for AVR (`TFT`, `Touch Panel`, `MMC`, `MP3`, `Serial Flash`, `Accel`, `UART`) z kodem `*_driver.c` / `resources.h`, ktory faktycznie dziala na tym sprzecie.

Potwierdzenia z tego zrodla:

- Kontroler LCD to na pewno `ILI9341` - jawne wywolanie `TFT_Init_ILI9341_8bit(240, 320)` w `TFT_driver.c`.
- Sygnaly `TFT_RST/RS/CS/RD/WR` sa **identyczne we wszystkich 6 projektach vendor**: `RST=PJ1`, `RD=PJ2`, `WR=PJ3`, `RS=PJ4`, `CS=PJ5`. Nasz `firmware/board/mikromedia_xmega_pins.h` mial to przesuniete o caly bit (`CS=PJ6, RS=PJ5, WR=PJ4, RD=PJ3, RST=PJ2`) - wziete z OCR schematu, ktore okazalo sie bledne. To najprawdopodobniej prawdziwa przyczyna martwego ekranu przez cala te sesje, niezaleznie od poprawnego wyboru kontrolera i resetu.
- Sygnal `LCD-BLED` (podswietlenie) jest niespojny miedzy projektami vendor: 4 projekty (`MMC`, `MP3`, `Serial Flash`, `Touch Panel`) uzywaja `PC4`, a 2 projekty (`TFT`, `Accel`) uzywaja `PJ0`. Zaden nie zgadza sie z naszym wczesniejszym `PC3` (z OCR schematu). Przyjeto `PC4` (wiekszosc), do zweryfikowania empirycznie testem migania podswietlenia - jesli nie zadziala, nastepny kandydat to `PJ0`.
- Piny dotyku (`DriveX_Left=PA2`, `DriveX_Right=PQ2`, `DriveY_Up=PQ3`, `DriveY_Down=PA1`) zgadzaja sie dokladnie z naszym `mikromedia_xmega_pins.h` - tu bledu nie bylo.

Poprawka w `firmware/board/mikromedia_xmega_pins.h`: `MIKRO_LCD_CS_BIT=5`, `MIKRO_LCD_RS_BIT=4`, `MIKRO_LCD_WR_BIT=3`, `MIKRO_LCD_RD_BIT=2`, `MIKRO_LCD_RST_BIT=1`, `MIKRO_LCD_BLED_BIT=4` (nadal port `C`).

Rebuild + reflash po poprawce mapowania pinow:

- testy hostowe: `xmega_ui tests OK`, `esp_protocol tests OK`;
- rozmiar: `7540 bytes` flash, `604 bytes` RAM;
- flash przez PDI: zapis i weryfikacja `7540 bytes of flash verified`;
- do zweryfikowania na sprzecie: to jest teraz najbardziej prawdopodobna poprawna wersja - czy ekran nareszcie cos pokazuje, i czy podswietlenie (`PC4`) reaguje na heartbeat.

**Potwierdzone na sprzecie: ekran dziala.** Poprawka mapowania pinow (`CS/RS/WR/RD/RST` przesuniete o jeden bit) byla wlasciwa przyczyna martwego ekranu przez cala sesje.

Po potwierdzeniu dzialania usunieto diagnostyczny "heartbeat" na podswietleniu (`mikro_backlight_toggle`, wywolanie w `firmware/app/main.c`) - byl tylko tymczasowym obejsciem na czas szukania przyczyny.

Rebuild + reflash po usunieciu heartbeatu:

- testy hostowe: `xmega_ui tests OK`, `esp_protocol tests OK`;
- rozmiar: `7516 bytes` flash, `604 bytes` RAM;
- flash przez PDI: zapis i weryfikacja `7516 bytes of flash verified`.

## Automatyczny obrot ekranu wedlug ulozenia plytki

Plytka ma wbudowany 3-osiowy akcelerometr `ADXL345` na magistrali I2C `TWID` (`SDA=PD0`, `SCL=PD1` - stale przypisanie krzemu ATxmega128A1, potwierdzone w datasheet i w `docs/xMega/mikroC PRO for AVR/Accel/`). Nie wymaga dodatkowego okablowania.

Zaimplementowano 2 orientacje (landscape normalny / obrocony o 180 stopni), zgodnie z decyzja o zakresie (portrait wymagalby przeprojektowania `xmega_ui_layout.h` na 240x320 i zostal odlozony):

- `firmware/transport/i2c_twid.c` / `.h` - niskopoziomowy, odpytywany (polled) sterownik magistrali TWID (start/write/read/stop), 100 kHz.
- `firmware/drivers/accel/adxl345.c` / `.h` - sterownik ADXL345: inicjalizacja (weryfikacja `DEVID=0xE5`, full resolution +/-2g, 100 Hz), odczyt X/Y/Z. Adres `0x1D` i rejestry wziete z `docs/xMega/mikroC PRO for AVR/Accel/ACCEL_driver.c`.
- `firmware/board/mikromedia_xmega_adapter.c`: nowe funkcje `mikro_orientation_init()`, `mikro_orientation_update()`, `mikro_lcd_is_flipped()`. Decyzja "flipped" na podstawie osi Y ADXL345 z progiem histerezy `+/-150 LSB` (~600 mg), zeby nie przelaczac orientacji na plasko lezacej plytce. Zmiana orientacji przelacza rejestr `MADCTL` ILI9341 (`0x28` normalny / `0xE8` obrocony) przez nowa funkcje `ili9341_set_rotation()`.
- `firmware/app/main.c`: `mikro_orientation_init()` wywolywane raz na starcie; `mikro_orientation_update()` odpytywane co ~250 ms w petli glownej. Gdy ekran jest obrocony, wspolrzedne dotyku z `touch_resistive_poll()` sa odwracane (`W-1-x`, `H-1-y`) przed przekazaniem do `ui_touch_down()` - panel dotykowy jest fizycznie sztywno zwiazany ze szklem i nie wie o programowym `MADCTL`.

Niepotwierdzone na sprzecie (do zweryfikowania po obroceniu plytki o 180 stopni):

- czy os Y ADXL345 rzeczywiscie odpowiada za wykrycie "gora/dol" tego panelu - jesli obrot nie dziala albo dziala odwrotnie, zmienic os/znak w komentarzu przy `ORIENTATION_FLIP_THRESHOLD` w `mikromedia_xmega_adapter.c`;
- czy zmiana `MADCTL` sama wystarcza do przeorientowania juz narysowanej tresci (teoretycznie tak, kontroler skanuje GRAM zgodnie z aktualnym `MADCTL`), czy w praktyce potrzebny jest dodatkowy `ui_draw_full()` po zmianie orientacji.

Rebuild + reflash z obslugą akcelerometru:

- testy hostowe: `xmega_ui tests OK`, `esp_protocol tests OK`;
- rozmiar: `8284 bytes` flash, `605 bytes` RAM;
- flash przez PDI: zapis i weryfikacja `8284 bytes of flash verified`.

**Potwierdzone na sprzecie: obrot dziala.** Brakujacy element byl inny niz przewidziano w sekcji "niepotwierdzone" powyzej - os/prog akcelerometru byly OK od razu, ale sama zmiana `MADCTL` faktycznie **nie** przerysowuje juz narysowanej tresci na tym kontrolerze/paneli. Naprawka: `firmware/app/main.c` wywoluje teraz `ui_draw_full()` zawsze, gdy `mikro_orientation_update()` zglosi zmiane orientacji.

Diagnostyka po drodze: dodano tymczasowy odczyt X/Y/Z na ekranie (`ACC OK/FAIL X.. Y.. Z..`), zeby potwierdzic dzialanie I2C bez podlaczonego terminala - usunieto po potwierdzeniu dzialania. Usunieto tez powiazane gettery `mikro_orientation_is_ready()`/`mikro_orientation_get_sample()` z adaptera (byly tylko do tego debugu).

## Podswietlenie: auto-wygaszanie po 10 s bezczynnosci

- `firmware/board/mikromedia_xmega_adapter.c`: nowa funkcja `mikro_backlight_set(uint8_t on)` (steruje `LCD-BLED`/`PC4`).
- `firmware/app/main.c`: kazdy wykryty dotyk (`touch_resistive_poll`) resetuje licznik bezczynnosci i wlacza podswietlenie, jesli bylo wygaszone. Po `BACKLIGHT_TIMEOUT_TICKS = 1000` iteracjach petli glownej bez dotyku (~10 s, petla spi 10 ms/iteracje) podswietlenie sie wylacza.
- Logika dotyku na przyciskach (hit-test, wysylka komend UART, kolorowanie stanu) byla juz zaimplementowana wczesniej w `firmware/ui/xmega_ui.c` (`ui_touch_down`) i pokryta testem `tests/test_xmega_ui.c` - nie wymagalo dodatkowej pracy.

Rebuild + reflash:

- testy hostowe: `xmega_ui tests OK`, `esp_protocol tests OK`;
- rozmiar: `8392 bytes` flash, `605 bytes` RAM;
- flash przez PDI: zapis i weryfikacja `8392 bytes of flash verified`;
- do zweryfikowania na sprzecie: czy podswietlenie faktycznie gasnie po ~10 s bezczynnosci i wraca natychmiast po dotyku; czy dotyk trafia we wlasciwe przyciski we obu orientacjach ekranu.

## Dotyk: diagnostyka przez USB-UART, poprawki kalibracji, menu + EEPROM

Po pierwszych testach na sprzecie dotyk nie trafial we wlasciwe przyciski. Zamiast dalej zgadywac przez male literki na LCD, dodano prawdziwa konsole diagnostyczna:

- `firmware/transport/uart_debug.c` / `.h` - `USARTF0` (`RXD0=PF2`, `TXD0=PF3`) do wbudowanego `FT232RL` (USB-UART), potwierdzone w datasheet (Table 30-6) i w `docs/xMega/mikroC PRO for AVR/UART/Uart.c` (`UARTF0_Init`). Wczesniejszy dokument mowil bledine `PF0/PF1` (to sa piny `TWIF`, nie `USART`) - ten sam typ bledu jak z pinami LCD.
- Na Macu widoczny jako `/dev/cu.usbserial-XXXXXXXX`. Pierwsza proba przy `115200` baud dala nieczytelne smieci - okazalo sie, ze to niedopasowanie predkosci (zegar/BSEL), nie zly pin. Przy `9600` baud dane sa czyste. `uart_debug_init()` w `main.c` uzywa `9600`.
- Odczyt na Macu: `stty -f /dev/cu.usbserial-XXXXXXXX 9600 raw -echo && cat /dev/cu.usbserial-XXXXXXXX`.

Dzieki temu namierzono realny problem z dotykiem:

1. **Detekcja "brak dotyku" byla wadliwa.** Stary prog (`raw` w oknie 80..4015) nie odrzucal stanu spoczynkowego - plytka w spoczynku czytala `raw_x≈3120, raw_y≈426`, co bylo *wewnatrz* tego okna, wiec system myslal, ze jest caly czas dotykany. Namierzono empirycznie: `raw_x` w spoczynku (~3120) jest zawsze znaczaco wyzszy niz przy jakimkolwiek realnym dotyku (obserwowane maks. ~2100) - nowy warunek `raw_x > TOUCH_NO_TOUCH_X_MIN(2700)` = brak dotyku, dziala niezaleznie od `raw_y` (ktorego wartosc w spoczynku nakladala sie na zakres realnych dotykow).
2. **Kalibracja `TOUCH_RAW_X/Y_MIN/MAX` byla zgadywana** i nie odpowiadala realnym zakresom ADC tej konkretnej plytki/panelu.
3. Zmiana `MADCTL` przy obrocie **nie przerysowuje** juz narysowanej tresci - patrz sekcja ponizej o `ui_draw_full()` po zmianie orientacji (znaleziono w tej samej rundzie diagnostyki co dotyk).

### Trwala kalibracja dotyku (EEPROM) + rozwijalne menu

Zamiast recznie wpisywac stale kalibracji, dodano prawdziwy mechanizm:

- `firmware/touch/touch_resistive.c`: `TOUCH_RAW_X/Y_MIN/MAX` sa teraz zmiennymi w RAM (nie `#define`), z wartosciami domyslnymi jako fallback. Nowe funkcje:
  - `touch_resistive_calibrate(raw_x1,screen_x1, raw_x2,screen_x2, raw_y1,screen_y1, raw_y2,screen_y2)` - liczy `raw_min/raw_max` z dwoch znanych punktow (przekatna: np. lewy-gorny i prawy-dolny cel) i zapisuje do EEPROM (`avr/eeprom.h`, `EEMEM`).
  - `touch_resistive_load_calibration()` - wczytuje z EEPROM przy starcie (magic `0xCA11`); jesli brak/niepoprawna - zwraca `0` i zostaja wartosci domyslne.
- `firmware/ui/xmega_menu.c` / `.h` - nowy, **rozwijalny** ekran menu (`MENU_ITEMS[]` - tablica `{label, action}`, latwo dodac kolejne pozycje w przyszlosci). Pierwsza pozycja: "Kalibracja dotyku" - rysuje 2 celowniki (przekatna), czeka na dotyk kazdego, liczy i zapisuje kalibracje.
- `firmware/app/main.c`:
  - Jesli EEPROM nie ma zapisanej kalibracji (pierwszy start), menu kalibracji odpala sie automatycznie zaraz po starcie.
  - W kazdej chwili: przytrzymanie dotyku w belce statusu (`y < UI_STATUS_H`, gdzie nie ma zadnego z 9 przyciskow) przez ~1.4 s otwiera menu recznie.
- `firmware/Makefile`: dodano `drivers/accel`... (juz bylo), `ui/xmega_menu.c`, `transport/uart_debug.c` do `SRCS`.
- Uwaga: `make flash` uzywa `avr-objcopy -R .eeprom`, czyli **nie nadpisuje EEPROM przy kazdym przeflashowaniu** - zapisana kalibracja przetrwa kolejne wgrania nowego firmware.

Rebuild + reflash z menu/kalibracja/EEPROM + poprawiona detekcja dotyku:

- testy hostowe: `xmega_ui tests OK`, `esp_protocol tests OK`;
- rozmiar: `10888 bytes` flash (7.8%), `739 bytes` RAM (9.0%), `10 bytes` EEPROM (magic + 4x `uint16`);
- flash przez PDI: zapis i weryfikacja `10888 bytes of flash verified`;
- do zweryfikowania na sprzecie: czy przy pierwszym starcie po tym flashu odpala sie kalibracja (EEPROM bylo puste), czy zapisana kalibracja daje poprawne trafienia we wszystkie 9 przyciskow, czy przytrzymanie belki statusu otwiera menu.

## Dotyk: znaleziona przyczyna zrodlowa, kalibracja po 9 przyciskach, EEPROM

Powyzsza sekcja diagnozowala symptomy; to jest znalezisko zrodlowe. Kalibracja "zapisywala sie" ale byla bledna, bo:

**Osie dotyku sa zamienione I odwrocone wzgledem ekranu.** Folia dotykowa na module ma wlasne elektryczne osie "X"/"Y" (nazwane tak w schemacie, sterowane przez `DRIVEA`/`DRIVEB`) obrocone o 90 stopni wzgledem obrazu LCD, i do tego dzialajace w odwrotnym kierunku (surowy odczyt ADC maleje, gdy wspolrzedna ekranowa rosnie). Potwierdzone jednoznacznie przez sweep kalibracyjny po 4, potem po 9 realnych przyciskach: dotyki dzielace ten sam ekranowy `Y` dawaly praktycznie identyczny odczyt na fazie "X" niezaleznie od `X`, i odwrotnie. Naprawka w `firmware/touch/touch_resistive.c`:

- `touch_last_raw_x`/`point->x` biora teraz odczyt z fazy "Y" (sterowanie `DRIVEB`, czytanie `XL`), a `touch_last_raw_y`/`point->y` z fazy "X" (sterowanie `DRIVEA`, czytanie `YD`) - odwrotnie niz pierwotnie.
- `map_axis()` liczy teraz na liczbach ze znakiem i nie zaklada `raw_min < raw_max` - obsluguje odwrocone osie.
- `touch_resistive_calibrate()` i `touch_resistive_load_calibration()` nie odrzucaja juz kalibracji tylko dlatego, ze `raw_at_0 > raw_at_max`.

**Kalibracja przez tylko 4 rogi siatki dawala niedopowiedzenia** (jeden nieprecyzyjny dotyk w rogu potrafil przejsc walidacje parami, ale skrzywic cala os - realny przypadek: zly dotyk "Sync" splaszczyl skale Y tak, ze dzialal tylko gorny rzad). Przepisano `firmware/ui/xmega_menu.c` tak, zeby prosic o dotkniecie **kazdego z 9 przyciskow po nazwie** ("Dotknij przycisk: Salon"), z walidacja zgodnosci calej trojki w kazdej kolumnie/rzedzie (nie tylko pary) przed zapisem - i retry calej sekwencji, jesli cokolwiek sie nie zgadza.

Inne poprawki znalezione w tej samej rundzie:

- **Detekcja "brak dotyku" byla zawsze prawdziwa** (stan spoczynkowy panelu wpadal w stare okno akceptacji) - naprawiono na prog oparty na realnie zmierzonym `raw` podczas fazy X.
- **Menu nie moglo zalezec od kalibracji, ktora ma naprawiac** - `menu_run()` uzywa teraz interakcji bez kalibracji (dotyk = uruchom, przytrzymanie = wyjdz), zamiast hit-testu po wierszach.
- **Przyciski jednorazowe (`All Off`, `Temp 1`, `Temp 2`, `Sync`) nigdy nie mialy zadnej wizualnej reakcji** (tylko `Salon..Noc` mialy stan PENDING) - dodano krotkie mignieccie w `firmware/app/main.c` jako potwierdzenie dotyku.
- **Przyciski `Salon..Noc` nie mialy prawdziwego przelaczania** - kazdy dotyk ustawial PENDING bez sprawdzenia obecnego stanu, wiec zapalony przycisk nie dal sie zgasic z panelu. Naprawiono w `firmware/ui/xmega_ui.c::ui_touch_down()`: dotyk teraz przelacza (OFF -> PENDING, cokolwiek-innego -> OFF).
- **Automatyczny obrot ekranu o 180 wylaczony na razie** (na prosbe uzytkownika, by zmniejszyc liczbe zmiennych podczas kalibracji dotyku) - kod w `mikromedia_xmega_adapter.c`/`lcd_ili9341.c` nadal istnieje, tylko nie jest wolany z `main.c`.
- **Diagnostyka przez USB-UART** (`firmware/transport/uart_debug.c`, `USARTF0`/`PF2`-`PF3`, potwierdzone w datasheet i `docs/xMega/.../UART/Uart.c`) - odczyt na Macu jako `/dev/cu.usbserial-XXXXXXXX` przy **9600 baud** (115200 dawalo smieci - niedopasowanie zegara/BSEL).

Rebuild + reflash po naprawie osi + 9-punktowej kalibracji + toggle:

- testy hostowe: `xmega_ui tests OK`, `esp_protocol tests OK`;
- rozmiar: `13056 bytes` flash (9.4%), `1353 bytes` RAM (16.5%), `10 bytes` EEPROM;
- flash przez PDI: zapis i weryfikacja `13056 bytes of flash verified`;
- potwierdzone na sprzecie: kalibracja przez 9 przyciskow zapisuje sie ze zgodnymi grupami (rozrzut rzedu dziesiatek jednostek), wszystkie 9 przyciskow reaguja na dotyk (5 z trwalym stanem, 4 z mignieciem), Salon..Noc przelaczaja sie (wlacz/wylacz) pod kolejnymi dotykami.

## Zmiana etykiet przelacznikow na realne idx Domoticza (2026-08-21)

Zamieniono placeholdery przyciskow (`Salon..Noc`, idx `101..105`) na 5 realnych urzadzen z Domoticza (potwierdzone live przez `json.htm?type=command&param=getdevices`):

- `1 Lozko Aga` (idx `32`), `2 Lozko Tomek` (idx `33`), `3 Biurko` (idx `34`, urzadzenie "Sypialnia Biurko"), `4 Dock Station` (idx `76`), `5 Salon Spr.` (idx `57`, urzadzenie "SALON SPRZET"). Wszystkie to `Light/Switch` typu `On/Off`, zgodne z komenda `Toggle`.
- Temperatury (`TEMP_IDX` w `domoticz_config.h` bez zmian: `39/40/42`): trzeci slot mial etykiete "Kuchnia", ale idx `44` ("Temperatura kuchnia") jest w Domoticzu `Used=0` i martwy od `2025-01-04` - za zgoda uzytkownika trzeci slot zostal opisany jako "Salon" (idx `39`, dziala) zamiast pokazywac zamrozony odczyt pod mylaca etykieta. Etykieta srodkowego slotu poprawiona z bledego "Kuchnia" na "Sypialnia" (odpowiadala idx `40`, ale nazwa w kodzie byla przekrecona od poczatku tej funkcji).
- Zaktualizowane pliki: `firmware/ui/xmega_ui_layout.h` (etykiety przyciskow), `firmware/ui/xmega_ui.c` (etykieta temperatury), `firmware/ui/xmega_menu.c` (komentarze przy stalych kalibracji), `esp/esp-domoticz-bridge/domoticz_config.h` (`LIGHT_IDX`), `ui/panel-preview.html` (makieta).
- Etykiety bez polskich znakow - font na `lcd_ili9341.c::glyph_column()` rysuje tylko `0-9`, `A-Z`, `. : -` i spacje; kazdy inny znak (w tym `l/o/z/ó/ę`) rysuje sie jako blok-placeholder `0x49`.

Rebuild + reflash:

- testy hostowe: `xmega_ui tests OK`, `esp_protocol tests OK`;
- rozmiar: `13090 bytes` flash (9.4%), `1387 bytes` RAM (16.9%), `10 bytes` EEPROM (kalibracja dotyku niezmieniona, bo `make flash` uzywa `-R .eeprom`);
- dostep USB do `AVRISP mkII` w tej sesji wymagal wylaczonego sandboxa Bash-a (bez tego: `Error: found but could not access USB device usb (03eb:2104)`, identyczny blad jak w pierwszej sesji ponizej - tam obejsciem bylo `sudo`, tutaj wystarczyl dostep bez sandboxa);
- flash przez PDI: zapis i weryfikacja `13090 bytes of flash verified`;
- odczyt po flashu: `Device signature = 1E 97 4C`, `Vtarget = 3.3 V` - plytka odpowiada poprawnie.

## Status "brak odpowiedzi ESP" na ekranie (2026-08-21)

Po zmianie idx przelacznikow uzytkownik zglosil, ze przyciski na panelu przelaczaja urzadzenie w Domoticzu niekonsekwentnie ("czasem wlacza, ale generalnie nie"). Wczesniej brak odpowiedzi z ESP po wyslaniu komendy `BTN,...` byl niewidoczny na LCD - status zostawal na "UART -> ESP" bez zadnej wskazowki, czy link UART XMEGA<->ESP w ogole zyje.

Dodano timeout w `firmware/app/main.c`: jesli po dotkniecie przycisku (kazde wywolanie `ui_touch_down()` zwracajace sukces) ESP nie odpowie zadna linia (`OK`/`ERR`/`STATE`/`TEMP`) w ciagu ok. 2 s (`ESP_WAIT_TIMEOUT_TICKS = 200` przy petli 10 ms/tik), status na ekranie zmienia sie na `ESP: BRAK ODP.`. To ma pomoc odroznic na sprzecie, bez podlaczania konsoli debug, czy problem jest po stronie UART/ESP (brak odpowiedzi) czy po stronie samego urzadzenia w Domoticzu (ESP odpowiada `OK`, ale przelacznik i tak sie nie zmienia).

Rebuild + reflash:

- testy hostowe: `xmega_ui tests OK`, `esp_protocol tests OK`;
- rozmiar: `13160 bytes` flash (9.4%), `1403 bytes` RAM (17.1%), `10 bytes` EEPROM (kalibracja bez zmian);
- flash przez PDI: zapis i weryfikacja `13160 bytes of flash verified`.

## Protokol XMEGA<->ESP oparty na idx Domoticza (2026-08-21)

Przeprojektowano protokol UART tak, zeby ESP w 100% odpowiadal za komunikacje
z Domoticz, a XMEGA znal idx urzadzen wprost i wysylal je jednym, spojnym
formatem: `idx:<idx>:1`/`idx:<idx>:0` (ustaw przelacznik), `idx:<idx>:?`
(zapytaj o wartosc), odpowiedz ESP `idx:<idx>:<wartosc>` albo
`ERR,IDX,<idx>`. Zastapiono nim stare tokeny `BTN,`/`GET,`/`STATE,`/`TEMP,`.

- Mapowanie przycisk/slot -> idx przeniesiono z ESP (`domoticz_config.h`,
  tablice `LIGHT_IDX`/`TEMP_IDX`) do nowego pliku `firmware/protocol/domoticz_map.h`
  po stronie XMEGA (`UI_SWITCH_IDX[5]`, `UI_TEMP_IDX[3]`, z realnymi idx
  `32/33/34/76/57` i `39/40/42`) - ESP nie zna juz zadnej logiki "przyciskow".
- `All Off` (przycisk 6) wysyla teraz po kolei 5 linii `idx:<idx>:0` (po jednej
  na kazdy przelacznik) zamiast jednej zbiorczej komendy `BTN,6,OFF_ALL`.
- `Sync` (przycisk 9) odswieza teraz takze stany przelacznikow, nie tylko
  temperatury - wysyla 8 zapytan `idx:<idx>:?` (5 przelacznikow + 3 czujniki).
- Zaktualizowane pliki: `firmware/protocol/esp_protocol.h/.c` (nowy typ
  komunikatu i generatory), `firmware/protocol/domoticz_map.h` (nowy),
  `firmware/ui/xmega_ui.c`, `esp/esp-domoticz-bridge/esp-domoticz-bridge.ino`,
  `esp/esp-domoticz-bridge/domoticz_config.h`/`.example.h` (usuniete
  `LIGHT_IDX`/`TEMP_IDX`), testy `tests/test_esp_protocol.c`,
  `tests/test_xmega_ui.c`, dokumentacja (`docs/project/architektura-firmware.md`,
  `esp/README.md`, `docs/project/domoticz-map.example.json`).

Rebuild + testy:

- testy hostowe: `xmega_ui tests OK`, `esp_protocol tests OK`;
- build AVR (`make -C firmware`) przechodzi bez warningow (`-Wall -Wextra`).

## Pierwszy test na zywym ESP (D1 mini) + niezawodnosc HTTP (2026-08-22)

Po wgraniu przeprojektowanego protokolu na fizyczny ESP8266 (D1 mini,
polaczony wlasnym USB do komputera, niezaleznie od XMEGA) przetestowano go
bezposrednio przez port szeregowy (`idx:<idx>:?`/`idx:<idx>:<0|1>`), bez
posredniczenia XMEGA:

- Pierwsza komenda po starcie (`idx:32:1`, zapalenie swiatla) zadzialala
  natychmiast. Kolejne wywolania (odczyty temperatur, potem `idx:32:0`)
  zaczely sporadycznie konczyc sie `ERR,IDX,<idx>`, mimo ze te same zapytania
  wyslane bezposrednio z komputera do tego samego Domoticza dzialaly zawsze
  szybko i bezblednie - wskazywalo to na problem po stronie ESP, nie w
  protokole ani w Domoticzu.
- Przyczyna: domyslny tryb oszczedzania energii WiFi na ESP8266 (modem
  sleep) opoznia/gubi pakiety po pierwszym polaczeniu. Naprawiono w
  `esp-domoticz-bridge.ino::setup()`: `WiFi.setSleepMode(WIFI_NONE_SLEEP)` na
  ESP8266 (`WiFi.setSleep(false)` na ESP32).
- Dodatkowo dodano `delay(150)` po kazdej obsluzonej linii `idx:...` w
  `handleLine()` - XMEGA moze wyslac kilka linii pod rzad (All Off, Sync) bez
  przerwy na UART, a bez tego opoznienia kolejne polaczenia HTTP czasem nie
  zdazyly sie zwolnic (ten sam wzorzec co stare `delay(80..250)` w
  usunietych petlach po stronie ESP).
- Po obu poprawkach: pelny "burst" 8 zapytan pod rzad (5 przelacznikow + 3
  czujniki, symulacja przycisku Sync) wyslanych bez zadnej przerwy przeszedl
  bezblednie za pierwszym razem.
- Dodano tez wbudowany LED (GPIO2 na D1 mini, aktywny stanem niskim) jako
  prosty wskaznik polaczenia WiFi - swieci na stale gdy `wifiReady()`,
  sprawdzane w `setup()` i w kazdej iteracji `loop()`.

## Dwufazowe potwierdzenie komend + ponawianie wysylki (2026-08-22)

Zaobserwowano na sprzecie, ze pojedyncza komenda czasem nie doczekala sie
odpowiedzi w krotkim oknie (kilka sekund), a druga proba tej samej komendy
zadzialala - bez sposobu odroznienia "ESP nie dostal linii" od "ESP dostal,
ale Domoticz wolno odpowiada". Wprowadzono dwuetapowe potwierdzenie:

- ESP odsyla `RCV,<idx>` natychmiast po odebraniu i sparsowaniu linii
  `idx:...`, zanim wywola (potencjalnie wolne, do ~8s) zadanie HTTP do
  Domoticza; dopiero po nim wysyla finalny wynik `idx:<idx>:<wartosc>` albo
  `ERR,IDX,<idx>` jak dotychczas.
- XMEGA (`firmware/app/main.c`) rozroznia teraz dwie fazy oczekiwania:
  `ESP_PHASE_WAIT_RCV` (krotki timeout `ESP_RCV_TIMEOUT_TICKS` ~400ms - brak
  `RCV,` w tym czasie oznacza problem z samym UART/ESP, wiec ta sama komenda
  jest wysylana ponownie, do `ESP_MAX_SEND_ATTEMPTS = 3` razy) i
  `ESP_PHASE_WAIT_RESULT` (dluzszy timeout `ESP_RESULT_TIMEOUT_TICKS` ~8.5s
  na finalny wynik po otrzymaniu `RCV,`, bez ponawiania - ponowna wysylka nie
  pomoze, jesli problem jest po stronie Domoticza).
- Mechanizm dotyczy tylko komend pojedynczych (przelaczniki 1-5, Temp 1/2) -
  `ui_state_t` ma nowe pole `pending_command`/`pending_retryable`
  (`firmware/ui/xmega_ui.c`/`.h`) ustawiane przy takim dotyku. All Off/Sync
  wysylaja kilka linii na raz i nie sa objete retry (ESP przetwarza je
  sekwencyjnie/blokujaco, wiec RCV drugiej linii i tak przyszloby dopiero po
  zakonczeniu HTTP pierwszej - retry nic by tu nie dal).
- Zaktualizowane pliki: `firmware/protocol/esp_protocol.h/.c` (nowy typ
  `ESP_MSG_RECEIVED`, parsowanie `RCV,<idx>`), `esp/esp-domoticz-bridge/esp-domoticz-bridge.ino`
  (wysylka `RCV,` przed HTTP), `firmware/ui/xmega_ui.h/.c`, `firmware/app/main.c`,
  testy `tests/test_esp_protocol.c`/`tests/test_xmega_ui.c`.

Rebuild + testy:

- testy hostowe: `xmega_ui tests OK`, `esp_protocol tests OK`;
- build AVR (`make -C firmware`) przechodzi bez warningow.

## Nastepny test po stronie sprzetu

1. Upewnic sie, ze plytka mikromedia jest zasilona i AVRISP mkII widzi target power. Dioda statusu AVRISP mkII powinna byc zielona po podlaczeniu targetu.
2. Sprawdzic orientacje tasmy: czerwony pasek to pin 1.
3. Sprawdzic multimetrem, czy na pinie `VCC-3.3` przy `CN5` jest ok. 3.3 V wzgledem `GND`.
4. Uruchomic odczyt sygnatury:

```sh
sudo avrdude -c avrispmkII -p x128a1 -P usb -v
```

5. Jesli MCU jest rewizji D i zwykle `x128a1` nie odpowie, sprobowac:

```sh
sudo avrdude -c avrispmkII -p x128a1d -P usb -v
```

## Zrodla

- MikroElektronika CDN: `https://download.mikroe.com/documents/smart-displays/mikromedia/3/xmega/mikromedia-xmega-manual-v111b.pdf`
- MikroElektronika CDN: `https://download.mikroe.com/documents/smart-displays/mikromedia/3/xmega/mikromedia-xmega-schematic-v111b.pdf`
- MikroElektronika CDN: `https://download.mikroe.com/documents/smart-displays/mikromedia/3/xmega/mikromedia-xmega-pinout-v111b.pdf`
- Microchip AVRISP mkII: `https://www.microchip.com/en-us/development-tool/atavrisp2`
- Microchip datasheet: `https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-8067-8-and-16-bit-AVR-Microcontrollers-ATxmega64A1-ATxmega128A1_Datasheet.pdf`

## Projekt panelu Domoticz

- Cel i zalozenia: `docs/project/cel-domoticz-esp-ui.md`
- Podglad interfejsu 320x240: `ui/panel-preview.html`
- Koordynaty 9 kontrolek pod firmware: `firmware/ui/xmega_ui_layout.h`
- Logika UI pod XMEGA: `firmware/ui/xmega_ui.c`, `firmware/ui/xmega_ui.h`
- Wejscie programu: `firmware/app/main.c`
- Mapa pinow LCD/touch/UART: `docs/project/pinout-sprzetowy.md`, `firmware/board/mikromedia_xmega_pins.h`
- Opis architektury firmware: `docs/project/architektura-firmware.md`
- Adapter sprzetowy: `firmware/board/mikromedia_xmega_adapter.c`
- Sterownik LCD ILI9341: `firmware/drivers/lcd/lcd_ili9341.c`
- Odczyt dotyku rezystancyjnego: `firmware/touch/touch_resistive.c`
- Akcelerometr (obrot ekranu): `firmware/drivers/accel/adxl345.c`, magistrala I2C `firmware/transport/i2c_twid.c`
- UART do ESP: `firmware/transport/uart_esp.c`
- Konsola diagnostyczna (USB-UART/FT232RL): `firmware/transport/uart_debug.c`
- Menu na urzadzeniu (kalibracja dotyku i przyszle funkcje): `firmware/ui/xmega_menu.c`
- Protokol tekstowy ESP: `firmware/protocol/esp_protocol.c`
- Mapowanie idx Domoticza (zrodlo prawdy): `firmware/protocol/domoticz_map.h`
- Szkic mostka ESP -> Domoticz: `esp/esp-domoticz-bridge/esp-domoticz-bridge.ino`
- Przykladowa konfiguracja ESP: `esp/esp-domoticz-bridge/domoticz_config.example.h`
- Przykladowe mapowanie `idx`: `docs/project/domoticz-map.example.json`
- Test hostowy logiki UI: `tests/test_xmega_ui.c`
