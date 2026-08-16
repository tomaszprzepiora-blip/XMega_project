# Cel projektu: panel XMEGA + ESP + Domoticz

## Pomysl

Zbudowac maly panel sterowania na plytce `mikromedia for XMEGA`.

Panel ma:

- wyswietlac prosty interfejs na TFT 320x240,
- miec 9 dotykowych kontrolek na dole ekranu,
- po nacisnieciu kontrolki wysylac komende do modulu ESP,
- przez ESP sterowac Domoticzem, np. wlacz/wylacz swiatlo,
- odbierac z ESP odczyty temperatur i wyswietlac je na gorze ekranu.

## Podzial odpowiedzialnosci

- `ATxmega128A1`: ekran, dotyk, lokalny stan UI, wysylanie prostych komend do ESP.
- `ESP`: Wi-Fi, HTTP/MQTT/API Domoticza, pobieranie temperatur.
- `Domoticz`: docelowa automatyka, przelaczniki, sensory temperatury.

## Pierwszy uklad ekranu

Ekran: `320x240`, orientacja landscape.

- `0..95 px`: status, zegar, temperatury, ostatnia odpowiedz ESP.
- `96..239 px`: 9 kontrolek dotykowych w siatce `3 x 3`.

Kontrolki:

1. `Salon`
2. `Kuchnia`
3. `Korytarz`
4. `Biurko`
5. `Noc`
6. `All Off`
7. `Temp 1`
8. `Temp 2`
9. `Sync`

Nazwy sa robocze. Po podaniu indeksow Domoticza mozna je zamienic na konkretne urzadzenia.

## Protokol XMEGA <-> ESP, wersja szkicowa

UART, tekst ASCII, jedna komenda w jednej linii zakonczonej `\n`.

Przyciski:

```text
BTN,1,TOGGLE
BTN,2,TOGGLE
BTN,6,OFF_ALL
```

Zapytanie o temperatury:

```text
GET,TEMP
```

Odpowiedz ESP:

```text
TEMP,1,21.6
TEMP,2,19.8
STATE,1,ON
STATE,2,OFF
OK
ERR,opis
```

## Nastepne dane potrzebne od strony Domoticza

- adres IP/host Domoticza,
- port,
- czy jest login/haslo,
- `idx` urzadzen dla swiatel,
- `idx` sensorow temperatury,
- czy ESP ma uzywac HTTP JSON API czy MQTT.

## Artefakty w workspace

- `ui/panel-preview.html` - klikalna makieta ekranu 320x240.
- `firmware/xmega_ui_layout.h` - geometria 9 kontrolek.
- `firmware/xmega_ui.c` i `firmware/xmega_ui.h` - logika UI, stan, parser odpowiedzi ESP.
- `esp/esp-domoticz-bridge/esp-domoticz-bridge.ino` - szkic ESP8266/ESP32 jako most UART do Domoticza.
- `tests/test_xmega_ui.c` - test hostowy logiki UI.
- `docs/project/pinout-sprzetowy.md` - mapa LCD/touch/UART z dokumentacji.
- `docs/project/domoticz-map.example.json` - robocze mapowanie kontrolek na `idx`.
- `esp/esp-domoticz-bridge/domoticz_config.example.h` - konfiguracja ESP do skopiowania jako `domoticz_config.h`.

## API Domoticza

Aktualna dokumentacja Domoticza ostrzega, ze od stable 2025.1 stare formy typu `type=devices` sa usuniete. Dlatego szkic ESP uzywa nowszej postaci:

```text
/json.htm?type=command&param=switchlight&idx=99&switchcmd=Toggle
/json.htm?type=command&param=getdevices&rid=201
```

Zrodlo: `https://wiki.domoticz.com/Domoticz_API/JSON_URL%27s`
