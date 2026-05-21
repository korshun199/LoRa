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

PROJECT_NAME="${PROJECT_NAME:-LoRa}"
SERVER_HOST="${SERVER_HOST:-0.0.0.0}"
SERVER_PORT="${SERVER_PORT:-8000}"

if [ -z "$SERVER_HOST" ]; then
    SERVER_HOST="0.0.0.0"
fi

if [ -z "$SERVER_PORT" ]; then
    SERVER_PORT="8000"
fi

cd "$SERVER_DIR"

if [ ! -d "$SERVER_DIR/venv" ]; then
    echo "venv not found, creating..."
    python3 -m venv venv
fi

source "$SERVER_DIR/venv/bin/activate"

pip install -r "$SERVER_DIR/requirements.txt"

echo "Starting $PROJECT_NAME server on $SERVER_HOST:$SERVER_PORT"
uvicorn app.main:app --host "$SERVER_HOST" --port "$SERVER_PORT" --reload
