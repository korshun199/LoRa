#!/usr/bin/env bash
set -euo pipefail

BAUD=115200

echo "=== БРАТАН Linux LoRa Monitor ==="

PORTS=(/dev/ttyACM* /dev/ttyUSB*)
FOUND=()

for p in "${PORTS[@]}"; do
    [ -e "$p" ] && FOUND+=("$p")
done

if [ "${#FOUND[@]}" -eq 0 ]; then
    echo "Нет плат. Воткни XIAO в USB. Да, именно в USB, не в судьбу."
    exit 1
fi

echo "Найдено:"
for i in "${!FOUND[@]}"; do
    echo "$((i+1))) ${FOUND[$i]}"
done

if [ "${#FOUND[@]}" -eq 1 ]; then
    PORT="${FOUND[0]}"
else
    read -rp "Номер порта: " N
    PORT="${FOUND[$((N-1))]}"
fi

echo
echo "Открываю $PORT на $BAUD"
echo "Выход: Ctrl+C"
echo

python3 - <<PY
import serial, time

port = "$PORT"
baud = $BAUD

while True:
    try:
        with serial.Serial(port, baud, timeout=1) as ser:
            print(f"[OK] Слушаю {port}")
            time.sleep(1)
            while True:
                line = ser.readline()
                if line:
                    print(line.decode("utf-8", errors="replace").rstrip())
    except Exception as e:
        print(f"[reconnect] {e}")
        time.sleep(2)
PY
