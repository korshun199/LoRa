#!/usr/bin/env bash

set -e

BASE_URL="${1:-http://127.0.0.1:8000}"
GATEWAY_ID="gateway-001"
NODE_ID="node-001"

echo "=== LoRa API cycle test ==="
echo "BASE_URL: $BASE_URL"
echo

echo "=== 1. Health check ==="
curl -s "$BASE_URL/health"
echo
echo

echo "=== 2. Register gateway ==="
curl -s -X POST "$BASE_URL/api/devices/register" \
  -H "Content-Type: application/json" \
  -d "{\"device_id\":\"$GATEWAY_ID\",\"device_type\":\"gateway\",\"firmware_version\":\"0.1.0\"}"
echo
echo

echo "=== 3. Create command for node ==="
COMMAND_RESPONSE="$(curl -s -X POST "$BASE_URL/api/commands" \
  -H "Content-Type: application/json" \
  -d "{\"target_id\":\"$NODE_ID\",\"command\":\"relay_set\",\"channel\":1,\"value\":\"on\"}")"

echo "$COMMAND_RESPONSE"
echo

MSG_ID="$(python3 -c 'import json,sys; print(json.load(sys.stdin)["command"]["msg_id"])' <<< "$COMMAND_RESPONSE")"

echo
echo "MSG_ID: $MSG_ID"
echo

echo "=== 4. Read command as node/gateway ==="
curl -s "$BASE_URL/api/devices/$NODE_ID/command"
echo
echo

echo "=== 5. Send ACK with real msg_id ==="
curl -s -X POST "$BASE_URL/api/devices/$NODE_ID/command/ack" \
  -H "Content-Type: application/json" \
  -d "{\"device_id\":\"$NODE_ID\",\"msg_id\":\"$MSG_ID\",\"result\":\"done\",\"message\":\"relay 1 switched on\"}"
echo
echo

echo "=== 6. Debug state ==="
curl -s "$BASE_URL/api/debug/state"
echo
echo

echo "=== Test finished ==="
