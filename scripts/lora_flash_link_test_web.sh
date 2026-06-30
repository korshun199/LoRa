#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="/home/work/LoRa"
FW_DIR="$PROJECT_ROOT/firmware/xiao_lora_link_test_web"
BAUD="115200"

green() { printf "\033[32m%s\033[0m\n" "$*"; }
yellow() { printf "\033[33m%s\033[0m\n" "$*"; }
red() { printf "\033[31m%s\033[0m\n" "$*"; }

find_port() {
  local ports=()
  while IFS= read -r p; do ports+=("$p"); done < <(ls /dev/ttyACM* /dev/ttyUSB* 2>/dev/null || true)

  if [ "${#ports[@]}" -eq 0 ]; then
    return 1
  fi

  if [ "${#ports[@]}" -eq 1 ]; then
    echo "${ports[0]}"
    return 0
  fi

  yellow "Найдено несколько портов:"
  local i=1
  for p in "${ports[@]}"; do
    echo "  $i) $p"
    i=$((i+1))
  done

  read -rp "Выбери номер порта: " n
  echo "${ports[$((n-1))]}"
}

send_config() {
  local port="$1"
  local callsign="$2"
  local ip="$3"

  python3 - "$port" "$BAUD" "$callsign" "$ip" <<'PY'
import sys
import time
import serial

port = sys.argv[1]
baud = int(sys.argv[2])
callsign = sys.argv[3]
ip = sys.argv[4]

cmd = f"CONFIG {callsign} {ip}\n"

print(f"[INFO] Opening {port} @ {baud}")
ser = serial.Serial(port, baudrate=baud, timeout=0.5)
time.sleep(2.0)

print(f"[INFO] Sending: {cmd.strip()}")
ser.write(cmd.encode("utf-8"))
ser.flush()

deadline = time.time() + 8
buf = ""

while time.time() < deadline:
    data = ser.read(256)
    if data:
        text = data.decode("utf-8", errors="replace")
        print(text, end="")
        buf += text
        if "CONFIG_OK" in buf:
            ser.close()
            sys.exit(0)
        if "CONFIG_ERROR" in buf:
            ser.close()
            sys.exit(2)

ser.close()
print("\n[ERROR] No CONFIG_OK received")
sys.exit(1)
PY
}

echo
green "=== LoRa Link Test Web Flasher ==="
echo

cd "$FW_DIR"

yellow "[1/2] Сборка прошивки..."
pio run

count=1

while true; do
  echo
  green "=== Плата #$count ==="
  yellow "Подключи одну плату и нажми Enter."
  read -r _

  if ! port="$(find_port)"; then
    red "Порт не найден. Подключи плату и попробуй снова."
    continue
  fi

  read -rp "Позывной, например BRATAN: " callsign
  read -rp "IP из 192.168.4.0/24, например 192.168.4.10: " ip

  if [ -z "$callsign" ] || [ -z "$ip" ]; then
    red "Позывной и IP не должны быть пустыми."
    continue
  fi

  yellow "[2/2] Прошиваю $callsign / $ip на $port ..."
  pio run -t upload --upload-port "$port"

  yellow "Жду перезагрузку платы..."
  sleep 3

  yellow "Записываю настройки через Serial..."
  send_config "$port" "$callsign" "$ip"

  green "[OK] Готово: $callsign / $ip"
  echo

  echo "Что дальше?"
  echo "  1) Прошить ещё одну плату"
  echo "  2) Выход"
  read -rp "Выбор: " choice

  case "$choice" in
    1)
      count=$((count+1))
      ;;
    2)
      green "Выход. Конвейер остановлен."
      exit 0
      ;;
    *)
      yellow "Не понял выбор, считаю что выход. Люди и меню, вечная трагедия."
      exit 0
      ;;
  esac
done
