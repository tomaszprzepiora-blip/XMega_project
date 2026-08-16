# Pinout sprzetowy mikromedia for XMEGA

Zrodlo: `docs/mikromedia-xmega-schematic-v111b.pdf`, schemat v1.11.

## LCD / TFT

Modul wyswietlacza: `MI0283QT2`, TFT `320x240`, `262144` kolory, panel dotykowy rezystancyjny.

Magistrala danych LCD:

| Net | Pin XMEGA |
| --- | --- |
| `T-D0` | `PK0` |
| `T-D1` | `PK1` |
| `T-D2` | `PK2` |
| `T-D3` | `PK3` |
| `T-D4` | `PK4` |
| `T-D5` | `PK5` |
| `T-D6` | `PK6` |
| `T-D7` | `PK7` |

Sterowanie LCD:

| Net | Pin XMEGA | Znaczenie |
| --- | --- | --- |
| `LCD-CS#` | `PJ6` | chip select |
| `LCD-RS` | `PJ5` | command/data |
| `PMWR` | `PJ4` | write strobe |
| `PMRD` | `PJ3` | read strobe |
| `LCD-RST` | `PJ2` | reset LCD |
| `LCD-BLED` | `PC3` | backlight |

## Dotyk rezystancyjny

| Net | Pin XMEGA / funkcja |
| --- | --- |
| `LCD-XL` | `PA2` |
| `LCD-YD` | `PA1` |
| `DRIVEB` | `PQ3` |
| `DRIVEA` | `PQ2` |
| `LCD-XR` | przez obwod sterujacy na module TFT |
| `LCD-YU` | przez obwod sterujacy na module TFT |

Do odczytu dotyku trzeba skonfigurowac pary elektrod jako drive/sense i mierzyc ADC po kalibracji do ukladu `320x240`.

## UART do ESP

Najwygodniejsze wyprowadzenie na bocznym headerze `HDR2`:

| Header | Net | Pin XMEGA | Kierunek |
| --- | --- | --- | --- |
| `HDR2 pin 47` | `RXD-PD2` | `PD2` | XMEGA RX, laczyc z ESP TX |
| `HDR2 pin 48` | `TXD-PD3` | `PD3` | XMEGA TX, laczyc z ESP RX |
| `HDR2 pin 51` | `3.3V` | zasilanie/reference | uwaga na prad ESP |
| `HDR2 pin 52` | `GND` | masa | wspolna masa |

On-board `FT232RL` jest podpiety do `RXF-MCU` / `TXF-MCU` na `PF0` / `PF1`, wiec dla ESP lepiej zostawic `PD2/PD3`, zeby nie walczyc z USB-UART.

