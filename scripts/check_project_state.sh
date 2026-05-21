#!/usr/bin/env bash

set -e

PROJECT_ROOT="/home/work/LoRa"

cd "$PROJECT_ROOT"

echo "=== LoRa project state ==="
echo "PROJECT_ROOT: $PROJECT_ROOT"
echo

echo "=== Git branch ==="
git branch --show-current
echo

echo "=== Git status ==="
git status --short
echo

echo "=== Important files ==="
for file in \
  ".lora.env" \
  ".lora.env.example" \
  "server/app/main.py" \
  "server/app/core/config.py" \
  "server/requirements.txt" \
  "scripts/run_server_dev.sh" \
  "scripts/test_api_cycle.sh" \
  "docs/dev-server-mvp.md"
do
  if [ -f "$file" ]; then
    echo "OK   $file"
  else
    echo "MISS $file"
  fi
done

echo
echo "=== Server URLs ==="
echo "Health: http://127.0.0.1:8000/health"
echo "Docs:   http://127.0.0.1:8000/docs"
