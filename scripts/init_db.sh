#!/usr/bin/env bash

set -e

PROJECT_ROOT="/home/work/LoRa"
CONFIG_FILE="$PROJECT_ROOT/.lora.env"

if [ ! -f "$CONFIG_FILE" ]; then
    echo "[FAIL] Config not found: $CONFIG_FILE"
    exit 1
fi

source "$CONFIG_FILE"

DB_HOST="${DB_HOST:-127.0.0.1}"
DB_PORT="${DB_PORT:-3306}"
DB_NAME="${DB_NAME:-lora_db}"
DB_USER="${DB_USER:-lora_user}"
DB_PASSWORD="${DB_PASSWORD:-change_me_later}"

echo "=== LoRa DB init ==="
echo "DB_HOST: $DB_HOST"
echo "DB_PORT: $DB_PORT"
echo "DB_NAME: $DB_NAME"
echo "DB_USER: $DB_USER"
echo

mariadb -h "$DB_HOST" -P "$DB_PORT" -u "$DB_USER" -p"$DB_PASSWORD" "$DB_NAME" <<SQL
CREATE TABLE IF NOT EXISTS devices (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    device_id VARCHAR(128) NOT NULL,
    device_type VARCHAR(64) NOT NULL DEFAULT "node",
    firmware_version VARCHAR(64) NULL,
    status VARCHAR(64) NOT NULL DEFAULT "unknown",
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (id),
    UNIQUE KEY uq_devices_device_id (device_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS commands (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    msg_id VARCHAR(128) NOT NULL,
    target_id VARCHAR(128) NOT NULL,
    command VARCHAR(128) NOT NULL,
    value_text VARCHAR(255) NULL,
    channel INT NULL,
    status VARCHAR(64) NOT NULL DEFAULT "pending",
    ack_result VARCHAR(64) NULL,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    updated_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (id),
    UNIQUE KEY uq_commands_msg_id (msg_id),
    KEY idx_commands_target_status (target_id, status)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS statuses (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    device_id VARCHAR(128) NOT NULL,
    status VARCHAR(64) NOT NULL,
    message TEXT NULL,
    battery INT NULL,
    rssi INT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (id),
    KEY idx_statuses_device_created (device_id, created_at)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS acks (
    id BIGINT UNSIGNED NOT NULL AUTO_INCREMENT,
    msg_id VARCHAR(128) NOT NULL,
    device_id VARCHAR(128) NOT NULL,
    result VARCHAR(64) NOT NULL,
    message TEXT NULL,
    created_at TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (id),
    KEY idx_acks_msg_id (msg_id),
    KEY idx_acks_device_id (device_id)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
SQL

echo
echo "=== Tables ==="
mariadb -h "$DB_HOST" -P "$DB_PORT" -u "$DB_USER" -p"$DB_PASSWORD" "$DB_NAME" -e "SHOW TABLES;"

echo
echo "[OK] LoRa database initialized"
