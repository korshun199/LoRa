# LoRa Checkpoint 2026-06-16

## Commit
53602a7

## Infrastructure

- GitHub: OK
- FastAPI: OK
- MariaDB: OK
- SQLAlchemy: OK

## Database Integration

### Write Path

- devices -> MariaDB
- commands -> MariaDB
- acks -> MariaDB
- statuses -> MariaDB

### Read Path

- GET /api/devices/{device_id}/command
- source=mariadb
- memory fallback enabled

## Debug

- /api/debug/state
- /api/debug/db

## Diagnostics

Command:

./scripts/lora_test --all

Result:

OK: 29
FAIL: 0
STATUS: OK

## Current Architecture

register        -> RAM + MariaDB
create command  -> RAM + MariaDB
read command    -> MariaDB + RAM fallback
ack             -> RAM + MariaDB
status          -> RAM + MariaDB

## Next Step

Gradual migration from RAM storage to MariaDB storage.
