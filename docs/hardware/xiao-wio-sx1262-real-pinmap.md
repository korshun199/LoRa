# XIAO ESP32S3 + Wio SX1262 Real Pin Map

Date: 2026-06-27

## Real soldered mapping

| Wio SX1262 signal | XIAO ESP32S3 pin |
|---|---|
| MOSI | D10 |
| MISO | D9 |
| SCK | D8 |
| SW / ANT_SW | D4 |
| NSS / CS | D3 |
| RST / RESET | D2 |
| BUSY | D1 |
| DIO1 | D0 |

## Confirmed result

This mapping successfully initialized the SX1262:

```text
radio.begin -> 0
radio.standby -> 0
