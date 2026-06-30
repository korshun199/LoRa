# LoRa Link Test Web — checkpoint 2026-06-30

Branch: dev/lora-link-test-web  
GitHub: origin/dev/lora-link-test-web

## Purpose

Test firmware and web interface for checking LoRa link quality between XIAO ESP32S3 + Wio SX1262 nodes.

Each node runs the same firmware. During flashing, the operator sets:

- callsign
- AP IP address in 192.168.4.0/24

Configuration is stored in NVS and survives reboot.

## Confirmed hardware pinmap

Wio SX1262 -> XIAO ESP32S3:

- DIO1 -> D0
- BUSY -> D1
- RST -> D2
- NSS / CS -> D3
- ANT_SW / SW -> D4
- SCK -> D8
- MISO -> D9
- MOSI -> D10

## Confirmed devices

### OLEG

- Callsign: OLEG
- IP: 192.168.4.10
- Wi-Fi AP: LORA-OLEG

### VALERA

- Callsign: VALERA
- IP: 192.168.4.11
- Wi-Fi AP: LORA-VALERA

## Working features

- Wi-Fi AP per node
- Web interface on node IP
- API endpoint: /api/status
- API endpoint: /api/peers
- LoRa beacon transmit
- LoRa beacon receive
- Neighbor table
- RSSI display
- SNR display
- Link quality percentage
- RX packet count
- Lost packet count
- Last seen timer
- Online/offline peer status
- Stale peer cleanup

## Peer timeout logic

- Peer is online if last beacon age <= 15 seconds.
- Peer is offline if last beacon age > 15 seconds.
- Peer is removed from list if last beacon age > 60 seconds.

## Confirmed behavior

When VALERA is powered on, OLEG shows:

- online: true
- quality: 100

When VALERA is powered off, OLEG shows after timeout:

- online: false
- quality: 0

After drop timeout, VALERA disappears from /api/peers.

## Useful commands

Build:

```bash
cd /home/work/LoRa/firmware/xiao_lora_link_test_web
pio run
cd /home/work/LoRa
./scripts/lora_flash_link_test_web.sh
curl http://192.168.4.10/api/status
git push -u origin dev/lora-link-test-web
