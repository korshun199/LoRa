# LoRa Checkpoint - First LoRa TX

Date: 2026-06-27

First successful LoRa radio transmission from Seeed XIAO ESP32S3 + Wio SX1262.

Confirmed:

- `radio.begin -> 0`
- `radio.standby -> 0`
- `transmit -> 0`
- `PING #1`, `PING #2`, `PING #3` sent successfully

Firmware:

- `firmware/xiao_lora_ping_tx`

Real pin map:

- MOSI -> D10
- MISO -> D9
- SCK -> D8
- SW / ANT_SW -> D4
- NSS / CS -> D3
- RST / RESET -> D2
- BUSY -> D1
- DIO1 -> D0

Next phase:

- LoRa receiver firmware
- RSSI/SNR check
- merge radio layer into gateway firmware
