#!/usr/bin/env bash

set -e

PROJECT_ROOT="/home/work/LoRa"
SERVER_DIR="$PROJECT_ROOT/server"

echo "=== LoRa: настройка локального центрального сервера ==="

cd "$PROJECT_ROOT"

echo "=== Проверяю центральный конфиг ==="
if [ ! -f "$PROJECT_ROOT/.lora.env" ]; then
    cat > "$PROJECT_ROOT/.lora.env" <<'CFG'
# ==================================================
# LoRa Project Central Config
# Личный локальный конфиг. НЕ ДОБАВЛЯТЬ В GIT.
# ==================================================

PROJECT_NAME="LoRa"
PROJECT_ROOT="/home/work/LoRa"

GIT_USER="korshun199"
GIT_REPO_NAME="LoRa"
GIT_REMOTE_SSH="git@github.com:korshun199/LoRa.git"
GIT_BRANCH="main"

SERVER_FRAMEWORK="fastapi"
SERVER_HOST="0.0.0.0"
SERVER_PORT="8000"
SERVER_API_BASE_URL="http://127.0.0.1:8000"

VPS_HOST=""
VPS_USER=""
VPS_PROJECT_DIR="/opt/lora"

DB_ENGINE="mariadb"
DB_HOST="127.0.0.1"
DB_PORT="3306"
DB_NAME="lora_db"
DB_USER="lora_user"
DB_PASSWORD="change_me_later"

GATEWAY_NAME="gateway-001"
GATEWAY_WIFI_AP_PREFIX="LoRa-Setup"
GATEWAY_DEFAULT_IP="192.168.4.1"
GATEWAY_LORA_CHIP="SX1262"
GATEWAY_LORA_FREQ=""

NODE_PREFIX="node"
NODE_DEFAULT_ROLE="end_node"

OWNER_NAME="Олег"
ASSISTANT_NAME="Мира"
BEER_KIND="unknown"
PROJECT_MOOD="сначала скрипт, потом пиво"
CFG
    echo "Создан личный конфиг: .lora.env"
else
    echo "Конфиг уже есть: .lora.env"
fi

grep -qxF ".lora.env" "$PROJECT_ROOT/.gitignore" || echo ".lora.env" >> "$PROJECT_ROOT/.gitignore"
grep -qxF "server/venv/" "$PROJECT_ROOT/.gitignore" || echo "server/venv/" >> "$PROJECT_ROOT/.gitignore"

echo "=== Создаю структуру server/ ==="
mkdir -p "$SERVER_DIR/app/api"
mkdir -p "$SERVER_DIR/app/core"
mkdir -p "$SERVER_DIR/app/services"
mkdir -p "$SERVER_DIR/app/models"
mkdir -p "$SERVER_DIR/app/db"
mkdir -p "$PROJECT_ROOT/scripts"

cat > "$SERVER_DIR/requirements.txt" <<'REQ'
fastapi
uvicorn[standard]
python-dotenv
pydantic
REQ

cat > "$SERVER_DIR/app/__init__.py" <<'EOF_APP_INIT'
EOF_APP_INIT

cat > "$SERVER_DIR/app/core/__init__.py" <<'EOF_CORE_INIT'
EOF_CORE_INIT

cat > "$SERVER_DIR/app/api/__init__.py" <<'EOF_API_INIT'
EOF_API_INIT

cat > "$SERVER_DIR/app/services/__init__.py" <<'EOF_SERVICES_INIT'
EOF_SERVICES_INIT

cat > "$SERVER_DIR/app/models/__init__.py" <<'EOF_MODELS_INIT'
EOF_MODELS_INIT

cat > "$SERVER_DIR/app/db/__init__.py" <<'EOF_DB_INIT'
EOF_DB_INIT

cat > "$SERVER_DIR/app/core/config.py" <<'PY'
import os
from pathlib import Path
from dotenv import load_dotenv

PROJECT_ROOT = Path(os.getenv("LORA_PROJECT_ROOT", "/home/work/LoRa"))
CONFIG_FILE = PROJECT_ROOT / ".lora.env"

if CONFIG_FILE.exists():
    load_dotenv(CONFIG_FILE)

PROJECT_NAME = os.getenv("PROJECT_NAME", "LoRa")
SERVER_HOST = os.getenv("SERVER_HOST", "0.0.0.0")
SERVER_PORT = int(os.getenv("SERVER_PORT", "8000"))
SERVER_API_BASE_URL = os.getenv("SERVER_API_BASE_URL", "http://127.0.0.1:8000")

GATEWAY_NAME = os.getenv("GATEWAY_NAME", "gateway-001")
NODE_PREFIX = os.getenv("NODE_PREFIX", "node")

DB_ENGINE = os.getenv("DB_ENGINE", "mariadb")
DB_HOST = os.getenv("DB_HOST", "127.0.0.1")
DB_PORT = os.getenv("DB_PORT", "3306")
DB_NAME = os.getenv("DB_NAME", "lora_db")
DB_USER = os.getenv("DB_USER", "lora_user")
PY

cat > "$SERVER_DIR/app/main.py" <<'PY'
from datetime import datetime, timezone
from typing import Optional, Dict, Any

from fastapi import FastAPI
from pydantic import BaseModel

from app.core.config import (
    PROJECT_NAME,
    SERVER_API_BASE_URL,
    GATEWAY_NAME,
    DB_ENGINE,
    DB_HOST,
    DB_NAME,
)

app = FastAPI(
    title=f"{PROJECT_NAME} Central Server",
    description="Central VPS-compatible control server for LoRa gateway and distributed LoRa nodes.",
    version="0.1.0",
)

commands: Dict[str, Dict[str, Any]] = {}
statuses: Dict[str, Dict[str, Any]] = {}
acks: Dict[str, Dict[str, Any]] = {}


class RegisterRequest(BaseModel):
    device_id: str
    device_type: str = "gateway"
    activation_key: Optional[str] = None
    firmware_version: Optional[str] = None


class CommandRequest(BaseModel):
    target_id: str
    command: str
    value: Optional[str] = None
    channel: Optional[int] = None


class StatusRequest(BaseModel):
    device_id: str
    status: str
    message: Optional[str] = None
    battery: Optional[int] = None
    rssi: Optional[int] = None


class AckRequest(BaseModel):
    device_id: str
    msg_id: str
    result: str
    message: Optional[str] = None


def now_iso() -> str:
    return datetime.now(timezone.utc).isoformat()


@app.get("/")
def root():
    return {
        "project": PROJECT_NAME,
        "role": "central_server",
        "status": "running",
        "api": SERVER_API_BASE_URL,
    }


@app.get("/health")
def health():
    return {
        "status": "ok",
        "time": now_iso(),
        "db": {
            "engine": DB_ENGINE,
            "host": DB_HOST,
            "name": DB_NAME,
            "mode": "not_connected_yet",
        },
    }


@app.post("/api/devices/register")
def register_device(request: RegisterRequest):
    statuses[request.device_id] = {
        "device_id": request.device_id,
        "device_type": request.device_type,
        "status": "registered",
        "firmware_version": request.firmware_version,
        "updated_at": now_iso(),
    }

    return {
        "result": "registered",
        "device_id": request.device_id,
        "server_time": now_iso(),
    }


@app.post("/api/commands")
def create_command(request: CommandRequest):
    msg_id = f"cmd-{int(datetime.now(timezone.utc).timestamp())}"

    command_data = {
        "msg_id": msg_id,
        "target_id": request.target_id,
        "command": request.command,
        "value": request.value,
        "channel": request.channel,
        "created_at": now_iso(),
        "status": "pending",
    }

    commands[request.target_id] = command_data

    return {
        "result": "command_created",
        "command": command_data,
    }


@app.get("/api/devices/{device_id}/command")
def get_command(device_id: str):
    command = commands.get(device_id)

    if not command:
        return {
            "device_id": device_id,
            "command": None,
            "status": "no_command",
        }

    return {
        "device_id": device_id,
        "command": command,
    }


@app.post("/api/devices/{device_id}/status")
def update_status(device_id: str, request: StatusRequest):
    statuses[device_id] = {
        "device_id": device_id,
        "status": request.status,
        "message": request.message,
        "battery": request.battery,
        "rssi": request.rssi,
        "updated_at": now_iso(),
    }

    return {
        "result": "status_saved",
        "device_id": device_id,
    }


@app.post("/api/devices/{device_id}/command/ack")
def acknowledge_command(device_id: str, request: AckRequest):
    acks[request.msg_id] = {
        "device_id": device_id,
        "msg_id": request.msg_id,
        "result": request.result,
        "message": request.message,
        "updated_at": now_iso(),
    }

    if device_id in commands:
        commands[device_id]["status"] = "acknowledged"
        commands[device_id]["ack_result"] = request.result

    return {
        "result": "ack_saved",
        "device_id": device_id,
        "msg_id": request.msg_id,
    }


@app.get("/api/devices")
def list_devices():
    return {
        "gateway_default": GATEWAY_NAME,
        "devices": statuses,
    }


@app.get("/api/debug/state")
def debug_state():
    return {
        "commands": commands,
        "statuses": statuses,
        "acks": acks,
    }
PY

cat > "$PROJECT_ROOT/scripts/run_server_dev.sh" <<'SH'
#!/usr/bin/env bash

set -e

PROJECT_ROOT="/home/work/LoRa"
SERVER_DIR="$PROJECT_ROOT/server"
CONFIG_FILE="$PROJECT_ROOT/.lora.env"

if [ ! -f "$CONFIG_FILE" ]; then
    echo "Config not found: $CONFIG_FILE"
    exit 1
fi

source "$CONFIG_FILE"

cd "$SERVER_DIR"

if [ ! -d "$SERVER_DIR/venv" ]; then
    echo "venv not found, creating..."
    python3 -m venv venv
fi

source "$SERVER_DIR/venv/bin/activate"

pip install -r "$SERVER_DIR/requirements.txt"

echo "Starting $PROJECT_NAME server on $SERVER_HOST:$SERVER_PORT"
uvicorn app.main:app --host "$SERVER_HOST" --port "$SERVER_PORT" --reload
SH

chmod +x "$PROJECT_ROOT/scripts/run_server_dev.sh"

echo "=== Создаю Python venv ==="
cd "$SERVER_DIR"

if [ ! -d "$SERVER_DIR/venv" ]; then
    python3 -m venv venv
fi

source "$SERVER_DIR/venv/bin/activate"
pip install --upgrade pip
pip install -r requirements.txt

cd "$PROJECT_ROOT"

echo "=== Git commit/push ==="
git add .gitignore server scripts .lora.env.example 2>/dev/null || git add .gitignore server scripts
if git diff --cached --quiet; then
    echo "Нет изменений для коммита."
else
    git commit -m "Add local FastAPI central server skeleton"
fi

git push

echo
echo "=== Готово ==="
echo "Запуск сервера:"
echo "cd /home/work/LoRa"
echo "./scripts/run_server_dev.sh"
echo
echo "Проверка:"
echo "curl http://127.0.0.1:8000/health"
echo
echo "API docs:"
echo "http://127.0.0.1:8000/docs"
