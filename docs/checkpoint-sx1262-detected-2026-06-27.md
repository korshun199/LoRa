# LoRa Checkpoint - SX1262 Detected

Date: 2026-06-27

## Summary

The Wio SX1262 radio module was successfully detected and initialized from the Seeed XIAO ESP32S3.

This confirms that the real soldered pin mapping is correct and that the ESP32-S3 can communicate with the SX1262 over SPI.

## Result

- RadioLib `radio.begin()` result: `0`
- RadioLib `radio.standby()` result: `0`
- SX1262 status: detected and initialized

## Real soldered pin mapping

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

## PlatformIO firmware

Probe firmware:

```text
firmware/xiao_sx1262_probe_v3_real_pins
[PINS] DIO1=D0(1), BUSY=D1(2), NRST=D2(3), NSS=D3(4), ANT_SW=D4(5)
[SPI] SCK=D8(7), MISO=D9(8), MOSI=D10(9)
[ANT_SW] HIGH
[SPI] begin real soldered pins

[SX1262] try radio.begin TCXO=1.60
[SX1262] radio.begin -> 0
[OK]

[SUCCESS] SX1262 detected and initialized
[SX1262] standby -> 0
[OK]
