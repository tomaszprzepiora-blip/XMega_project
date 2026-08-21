# ESP bridge

`esp-domoticz-bridge/esp-domoticz-bridge.ino` to szkic dla ESP8266 albo ESP32.

Rola ESP - w 100% odpowiada za komunikacje z Domoticz, bez zadnej wiedzy o
"przyciskach" czy ukladzie ekranu (to zyje w `firmware/protocol/domoticz_map.h`
po stronie XMEGA):

- odbiera z XMEGA przez UART linie `idx:<idx>:<0|1>` (ustaw) albo
  `idx:<idx>:?` (zapytaj),
- laczy sie z Wi-Fi,
- odsyla natychmiast `RCV,<idx>` (przed HTTP - potwierdzenie odbioru linii),
- wysyla zapytanie HTTP do Domoticza dla podanego idx,
- odsyla do XMEGA wynik: `idx:<idx>:<wartosc>` albo `ERR,IDX,<idx>`.

Wbudowany LED (D1 mini: GPIO2) swieci sie na stale, gdy WiFi jest polaczone -
prosty wskaznik stanu polaczenia widoczny bez podlaczania monitora portu.

Przed wgraniem trzeba ustawic:

- skopiowac `domoticz_config.example.h` jako `domoticz_config.h`,
- ustawic `WIFI_SSID`,
- ustawic `WIFI_PASS`,
- ustawic `DOMOTICZ_HOST`,
- ustawic `DOMOTICZ_PORT`.

Domoticz JSON API dla swiatel:

```text
/json.htm?type=command&param=switchlight&idx=99&switchcmd=Toggle
/json.htm?type=command&param=switchlight&idx=99&switchcmd=On
/json.htm?type=command&param=switchlight&idx=99&switchcmd=Off
```

Odczyt urzadzenia/sensora:

```text
/json.htm?type=command&param=getdevices&rid=201
```

Zrodlo API: https://wiki.domoticz.com/Domoticz_API/JSON_URL%27s
