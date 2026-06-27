#!/usr/bin/env bash
set -euo pipefail

PROJECT_ROOT="/home/work/LoRa"
SIM_DIR="$PROJECT_ROOT/tools/node_simulator"

mkdir -p "$SIM_DIR"

echo "[OK] Node simulator already exists:"
echo "$SIM_DIR/node_simulator.py"
echo "$SIM_DIR/README.md"
