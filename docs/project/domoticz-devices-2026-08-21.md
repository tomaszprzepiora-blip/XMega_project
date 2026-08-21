# Domoticz device idx dump - 2026-08-21

Source:

```text
http://192.168.10.249:8080/json.htm?type=command&param=getdevices&filter=all&used=true&order=Name
```

Domoticz version reported by API: `2026.3 (build 18248)`.

## Switch / group candidates

| idx | Name | Type | SubType | SwitchType | Status |
| ---: | --- | --- | --- | --- | --- |
| 1 | Swiatlo w sypialni | Group |  |  | Mixed |
| 34 | Sypialnia Biurko | Light/Switch | Switch | On/Off | On |
| 32 | Lozko Aga | Light/Switch | Switch | On/Off | Off |
| 33 | Lozko Tomek | Light/Switch | Switch | On/Off | Off |
| 43 | HP-WoL | Lighting 2 | AC | Push On Button | On |
| 47 | Przedluzacz (Switch 1) | Light/Switch | Switch | On/Off | On |
| 48 | Przedluzacz (Switch 2) | Light/Switch | Switch | On/Off | On |
| 49 | Przedluzacz (Switch 3) | Light/Switch | Switch | On/Off | On |
| 50 | Przedluzacz (Switch 4) | Light/Switch | Switch | On/Off | On |
| 57 | SALON SPRZET | Light/Switch | Switch | On/Off | Off |
| 62 | Przy lozku | Light/Switch | Switch | On/Off | On |
| 67 | Przedluzacz 1 (Switch 1) | Light/Switch | Switch | On/Off | Off |
| 68 | Przedluzacz 1 (Switch 2) | Light/Switch | Switch | On/Off | Off |
| 69 | Przedluzacz 1 (Switch 3) | Light/Switch | Switch | On/Off | Off |
| 70 | Przedluzacz 1 (Switch 4) | Light/Switch | Switch | On/Off | Off |
| 71 | Komputer | Light/Switch | Switch | On/Off | Off |
| 76 | Dock Station | Light/Switch | Switch | On/Off | Off |

## Temperature candidates

| idx | Name | Type | SubType | Data |
| ---: | --- | --- | --- | --- |
| 39 | Salon | Temp | LaCrosse TX3 | 25.6 C |
| 40 | Sypialnia | Temp | LaCrosse TX3 | 25.7 C |
| 41 | Temperatura | Temp + Humidity + Baro | THB1 - BTHR918, BTHGN129 | 26.4 C, 97 %, 986 hPa |
| 42 | Temp Out | Temp | LaCrosse TX3 | 20.4 C |

## Current firmware constraints

The XMEGA panel currently has 5 persistent toggle buttons, followed by
`All Off`, `Temp 1`, `Temp 2`, and `Sync`.

Current ESP config uses temperature idx values `39` (Salon), `40` (Sypialnia),
and `42` (Temp Out). There is no active kitchen sensor: idx `44` ("Temperatura
kuchnia") exists but is marked `Used=0` in Domoticz and last reported on
2025-01-04, so `39` (Salon) is used as a stand-in for the third temperature
slot instead, labeled truthfully as "Salon" rather than "Kuchnia".

Switch idx values are now wired to real devices:

| Button | idx | Device |
| --- | ---: | --- |
| 1 | 32 | Lozko Aga |
| 2 | 33 | Lozko Tomek |
| 3 | 34 | Sypialnia Biurko |
| 4 | 76 | Dock Station |
| 5 | 57 | SALON SPRZET |
