#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="/home/work/LoRa"
SIM_DIR="$PROJECT_ROOT/tools/node_simulator"

mkdir -p "$SIM_DIR"

cat > "$SIM_DIR/node_simulator.py" <<'PY'
#!/usr/bin/env python3
import argparse
import json
import time
import urllib.request
import urllib.error
from datetime import datetime, timezone


def request_json(method, url, payload=None, timeout=10):
    data = None
    headers = {"Content-Type": "application/json"}

    if payload is not None:
        data = json.dumps(payload).encode("utf-8")

    req = urllib.request.Request(url, data=data, headers=headers, method=method)

    try:
        with urllib.request.urlopen(req, timeout=timeout) as resp:
            body = resp.read().decode("utf-8")
            return json.loads(body) if body else {}
    except urllib.error.HTTPError as exc:
        body = exc.read().decode("utf-8", errors="replace")
        raise RuntimeError(f"HTTP {exc.code}: {body}") from exc
    except urllib.error.URLError as exc:
        raise RuntimeError(f"Connection failed: {exc}") from exc


def now_iso():
    return datetime.now(timezone.utc).isoformat()


def register(base_url, device_id, firmware_version):
    return request_json("POST", f"{base_url}/api/devices/register", {
        "device_id": device_id,
        "device_type": "node_simulator",
        "firmware_version": firmware_version,
    })


def send_status(base_url, device_id, counter):
    battery = max(10, 95 - counter)
    rssi = -60 - (counter % 25)

    return request_json("POST", f"{base_url}/api/devices/{device_id}/status", {
        "device_id": device_id,
        "status": "online",
        "message": f"simulator heartbeat {counter}",
        "battery": battery,
        "rssi": rssi,
    })


def get_command(base_url, device_id):
    return request_json("GET", f"{base_url}/api/devices/{device_id}/command")


def ack_command(base_url, device_id, msg_id):
    return request_json("POST", f"{base_url}/api/devices/{device_id}/command/ack", {
        "device_id": device_id,
        "msg_id": msg_id,
        "result": "done",
        "message": "node simulator executed command",
    })


def main():
    parser = argparse.ArgumentParser(description="LoRa node simulator")
    parser.add_argument("--base-url", default="http://lora.systemio.ru")
    parser.add_argument("--device-id", default="node-sim-001")
    parser.add_argument("--firmware", default="sim-0.1.0")
    parser.add_argument("--interval", type=int, default=10)
    parser.add_argument("--once", action="store_true")
    args = parser.parse_args()

    print(f"[SIM] base_url={args.base_url}")
    print(f"[SIM] device_id={args.device_id}")

    print("[SIM] register")
    print(json.dumps(register(args.base_url, args.device_id, args.firmware), ensure_ascii=False))

    counter = 0

    while True:
        counter += 1

        print(f"[SIM] status #{counter} {now_iso()}")
        print(json.dumps(send_status(args.base_url, args.device_id, counter), ensure_ascii=False))

        print("[SIM] check command")
        command_response = get_command(args.base_url, args.device_id)
        print(json.dumps(command_response, ensure_ascii=False))

        command = command_response.get("command")
        if command and command.get("msg_id"):
            msg_id = command["msg_id"]
            print(f"[SIM] ack command {msg_id}")
            print(json.dumps(ack_command(args.base_url, args.device_id, msg_id), ensure_ascii=False))

        if args.once:
            break

        time.sleep(args.interval)


if __name__ == "__main__":
    main()
PY

chmod +x "$SIM_DIR/node_simulator.py"

cat > "$SIM_DIR/README.md" <<'MD'
# LoRa Node Simulator

Software simulator for a future ESP32/LoRa node.

Default API:

```text
http://lora.systemio.ru
