# ESP bridge

`esp-domoticz-bridge/esp-domoticz-bridge.ino` to szkic dla ESP8266 albo ESP32.

Rola ESP:

- odbiera komendy tekstowe z XMEGA przez UART,
- laczy sie z Wi-Fi,
- wysyla zapytania HTTP do Domoticza,
- odsyla do XMEGA statusy i temperatury.

Przed wgraniem trzeba ustawic:

- skopiowac `domoticz_config.example.h` jako `domoticz_config.h`,
- ustawic `WIFI_SSID`,
- ustawic `WIFI_PASS`,
- ustawic `DOMOTICZ_HOST`,
- ustawic `DOMOTICZ_PORT`,
- ustawic tablice `LIGHT_IDX`,
- ustawic tablice `TEMP_IDX`.

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
