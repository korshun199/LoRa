# LoRa Checkpoint - First Real Device Online

Date: 2026-06-27

## Summary

First real hardware device successfully connected to the production LoRa backend.

This is the first complete real-device command cycle:

ESP32-S3 -> Wi-Fi -> lora.systemio.ru -> FastAPI -> MariaDB -> command -> ESP32-S3 -> ACK -> MariaDB.

## Device

- Board: Seeed XIAO ESP32S3 + Wio SX1262
- Device ID: xiao-wio-6C98D8A172E0
- Firmware: wifi-heartbeat-0.1.0
- Server: http://lora.systemio.ru
- Port used during flashing: /dev/ttyACM0

## Confirmed

- USB detected
- ESP32-S3 flashed successfully
- Wi-Fi connected
- Device registered through production API
- Heartbeat/status saved to MariaDB
- Command received from server
- Command acknowledged by device
- Dashboard/overview show acknowledged command

## Last verified command

- Command: relay_set
- Value: on
- Result: acknowledged
- ACK result: done

## Current project state

- T16 development environment OK
- VPS production environment OK
- GitHub synchronized
- lora_test passes through production domain
- Node simulator available
- First real ESP32-S3 device online

## Next phase

Initialize SX1262 over SPI.

Goal:

- Detect SX1262
- Verify SPI wiring/pin mapping
- Initialize radio driver
- Read radio status
- Prepare first LoRa packet test
