#!/usr/bin/env python3
import glob
import sys
import threading
import time

try:
    import serial
except Exception as e:
    print(f"[ОШИБКА] pyserial не найден: {e}")
    sys.exit(1)

ports = sorted(glob.glob("/dev/ttyACM*"))

print("=== BRATAN / CHUVAK DUAL MONITOR ===")
for p in ports:
    print(f"порт: {p}")
print("Ctrl+C для выхода.")
print()

def reader(port):
    label = port.split("/")[-1]
    while True:
        try:
            with serial.Serial(port, 115200, timeout=1) as ser:
                time.sleep(0.5)
                while True:
                    line = ser.readline()
                    if line:
                        text = line.decode("utf-8", errors="replace").rstrip()
                        if text:
                            print(f"[{label}] {text}", flush=True)
        except Exception as e:
            print(f"[{label}] reconnect: {e}", flush=True)
            time.sleep(2)

for p in ports:
    threading.Thread(target=reader, args=(p,), daemon=True).start()

try:
    while True:
        time.sleep(1)
except KeyboardInterrupt:
    print("\nВыход.")
