# LoRa Checkpoint - Real Radio Link

Date: 2026-06-27

## Achieved

- VPS production server works: http://lora.systemio.ru
- FastAPI + MariaDB + nginx + systemd OK
- T16, VPS, GitHub synchronized
- lora_test OK 33 / FAIL 0
- XIAO ESP32S3 + Wio SX1262 hardware verified
- Real SX1262 pin map found
- SX1262 initialized: radio.begin -> 0
- First LoRa TX successful
- First LoRa RX successful
- Two real boards exchanged LoRa packets
- Bratan/Chuvak human-readable chat firmware started

## Real pin map

- MOSI -> D10
- MISO -> D9
- SCK -> D8
- SW / ANT_SW -> D4
- NSS / CS -> D3
- RST / RESET -> D2
- BUSY -> D1
- DIO1 -> D0

## Confirmed radio link

RX received:

- payload: PING #9 / #10 / #11 / #12 from xiao-wio
- RSSI: about -16 dBm
- SNR: about 11 dB
- frequency error: about 9.69 Hz

## Next step

Continue from Bratan/Chuvak firmware and build real LoRa node protocol.
