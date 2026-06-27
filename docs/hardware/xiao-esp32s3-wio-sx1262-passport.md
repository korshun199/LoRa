# XIAO ESP32S3 + Wio SX1262 Hardware Passport

Date: 2026-06-27

## Board

- MCU board: Seeed Studio XIAO ESP32S3
- LoRa board: Seeed Studio Wio SX1262
- Role: future LoRa gateway / node test board

## USB

- Port: /dev/ttyACM0
- USB mode: USB-Serial/JTAG
- Product detected:
  - USB JTAG/serial debug unit
  - seeed-xiao-s3

## ESP32-S3

- Chip: ESP32-S3 QFN56
- Revision: v0.2
- Features:
  - Wi-Fi
  - Bluetooth 5 LE
  - Dual Core + LP Core
  - 240 MHz
  - Embedded PSRAM 8 MB
- Crystal: 40 MHz

## Flash

- Manufacturer: c8
- Device: 4017
- Detected flash size: 8 MB
- Flash type: quad
- Flash voltage: 3.3 V

## MAC

- MAC: e0:72:a1:d8:98:6c

## Notes

The board was detected successfully with esptool 5.3.0 via /dev/ttyACM0.

Manual bootloader mode may be required:

1. Hold BOOT.
2. Press/release RESET or reconnect USB.
3. Release BOOT.
4. Run esptool command.

