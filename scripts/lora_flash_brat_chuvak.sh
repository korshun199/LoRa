#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="/home/work/LoRa"
FW_DIR="$PROJECT_ROOT/firmware/xiao_lora_brat_chuvak"
PIO="$PROJECT_ROOT/.venv-pio/bin/pio"

one_port() {
  mapfile -t ports < <(ls /dev/ttyACM* 2>/dev/null || true)
  if [ "${#ports[@]}" -ne 1 ]; then
    echo "[ОШИБКА] Оставь подключённой только одну плату."
    ls -l /dev/ttyACM* 2>/dev/null || true
    exit 1
  fi
  echo "${ports[0]}"
}

cd "$FW_DIR"

echo "=== BUILD ==="
"$PIO" run -e bratan
"$PIO" run -e chuvak

echo
echo "=== FLASH БРАТАН ==="
echo "Отключи обе платы. Подключи только плату БРАТАН."
read -rp "Готово? Enter: "
PORT="$(one_port)"
"$PIO" run -e bratan -t upload --upload-port "$PORT"

echo
echo "=== FLASH ЧУВАК ==="
echo "Отключи БРАТАНА. Подключи только плату ЧУВАК."
read -rp "Готово? Enter: "
PORT="$(one_port)"
"$PIO" run -e chuvak -t upload --upload-port "$PORT"

echo
echo "=== ГОТОВО ==="
echo "Теперь подключи обе платы и запусти:"
echo "/home/work/LoRa/scripts/lora_dual_monitor.sh"
