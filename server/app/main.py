import logging
from datetime import datetime, timezone
from typing import Optional, Dict, Any

from fastapi import FastAPI, Request
from pydantic import BaseModel

from app.db.database import check_database_connection
from app.db.repositories import save_device, save_command, save_ack, save_status, get_latest_command, get_db_debug_state, list_devices, get_latest_status, get_status_history, get_command_history, get_device_overview
from app.core.config import (
    PROJECT_NAME,
    SERVER_API_BASE_URL,
    GATEWAY_NAME,
    DB_ENGINE,
    DB_HOST,
    DB_NAME,
)

logging.basicConfig(
    level=logging.INFO,
    format="%(asctime)s | %(levelname)s | LoRaServer | %(message)s",
)

logger = logging.getLogger("lora_server")

app = FastAPI(
    title=f"{PROJECT_NAME} Central Server",
    description="Central VPS-compatible control server for LoRa gateway and distributed LoRa nodes.",
    version="0.1.1",
)



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


@app.middleware("http")
async def log_requests(request: Request, call_next):
    client_host = request.client.host if request.client else "unknown"
    logger.info("HTTP IN  | %s %s | from=%s", request.method, request.url.path, client_host)

    response = await call_next(request)

    logger.info(
        "HTTP OUT | %s %s | status=%s",
        request.method,
        request.url.path,
        response.status_code,
    )

    return response


@app.on_event("startup")
def on_startup():
    logger.info("SERVER START | project=%s | api=%s", PROJECT_NAME, SERVER_API_BASE_URL)
    logger.info(
        "CONFIG       | db_engine=%s | db_host=%s | db_name=%s | gateway_default=%s",
        DB_ENGINE,
        DB_HOST,
        DB_NAME,
        GATEWAY_NAME,
    )
    db_ok, db_info = check_database_connection()
    if db_ok:
        logger.info("DATABASE     | MariaDB connection OK | version=%s", db_info)
    else:
        logger.error("DATABASE     | MariaDB connection FAILED | error=%s", db_info)

    logger.info("MODE         | storage=in_memory | maria_db=connection_checked")


@app.get("/")
def root():
    logger.info("ROOT         | server info requested")

    return {
        "project": PROJECT_NAME,
        "role": "central_server",
        "status": "running",
        "api": SERVER_API_BASE_URL,
        "storage": "in_memory",
    }


@app.get("/health")
def health():
    logger.info("HEALTH       | ok")

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
    logger.info(
        "REGISTER     | device_id=%s | type=%s | firmware=%s | activation_key=%s",
        request.device_id,
        request.device_type,
        request.firmware_version,
        "provided" if request.activation_key else "empty",
    )

    # Device registration storage is now handled by MariaDB.
    # RAM storage for registered devices is intentionally disabled.

    try:
        save_device(
            device_id=request.device_id,
            device_type=request.device_type,
            firmware_version=request.firmware_version,
        )
        logger.info("DB DEVICE    | saved device_id=%s", request.device_id)
    except Exception as exc:
        logger.error("DB DEVICE    | save failed device_id=%s | error=%s", request.device_id, exc)

    logger.info("REGISTER OK  | device_id=%s | storage=mariadb", request.device_id)

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

    # Command storage is now handled by MariaDB.
    # RAM storage for commands is intentionally disabled.

    try:
        save_command(
            msg_id=msg_id,
            target_id=request.target_id,
            command=request.command,
            value_text=request.value,
            channel=request.channel,
        )
        logger.info("DB COMMAND   | saved msg_id=%s | target=%s", msg_id, request.target_id)
    except Exception as exc:
        logger.error("DB COMMAND   | save failed msg_id=%s | error=%s", msg_id, exc)

    logger.info(
        "COMMAND NEW  | msg_id=%s | target=%s | command=%s | channel=%s | value=%s",
        msg_id,
        request.target_id,
        request.command,
        request.channel,
        request.value,
    )

    return {
        "result": "command_created",
        "command": command_data,
    }


@app.get("/api/devices/{device_id}/overview")
def get_device_overview_api(device_id: str):
    try:
        overview = get_device_overview(device_id)
        logger.info(
            "DEVICE VIEW  | device_id=%s | status_count=%s | command_count=%s",
            device_id,
            overview.get("status_count"),
            overview.get("command_count"),
        )
        return overview
    except Exception as exc:
        logger.error("DEVICE VIEW  | failed device_id=%s | error=%s", device_id, exc)
        return {
            "device_id": device_id,
            "storage": "mariadb",
            "status": "failed",
            "error": str(exc),
        }



@app.get("/api/devices/{device_id}/commands/history")
def get_device_command_history(device_id: str, limit: int = 20):
    try:
        history = get_command_history(device_id, limit)
        logger.info("COMMAND HIST | device_id=%s | count=%s", device_id, len(history))

        return {
            "device_id": device_id,
            "storage": "mariadb",
            "count": len(history),
            "history": history,
        }
    except Exception as exc:
        logger.error("COMMAND HIST | failed device_id=%s | error=%s", device_id, exc)
        return {
            "device_id": device_id,
            "storage": "mariadb",
            "status": "failed",
            "error": str(exc),
        }



@app.get("/api/devices/{device_id}/command")
def get_command(device_id: str):
    try:
        db_command = get_latest_command(device_id)
    except Exception as exc:
        logger.error("DB COMMAND   | read failed device_id=%s | error=%s", device_id, exc)
        db_command = None

    if db_command:
        command = {
            "msg_id": db_command.get("msg_id"),
            "target_id": db_command.get("target_id"),
            "command": db_command.get("command"),
            "value": db_command.get("value_text"),
            "channel": db_command.get("channel"),
            "status": db_command.get("status"),
            "ack_result": db_command.get("ack_result"),
        }

        logger.info(
            "DB COMMAND   | read device_id=%s | msg_id=%s | status=%s",
            device_id,
            command.get("msg_id"),
            command.get("status"),
        )

        return {
            "device_id": device_id,
            "command": command,
            "source": "mariadb",
        }

    logger.info("COMMAND GET  | device_id=%s | result=no_command", device_id)

    return {
        "device_id": device_id,
        "command": None,
        "status": "no_command",
        "source": "mariadb",
    }


@app.get("/api/devices/{device_id}/status/history")
def get_device_status_history(device_id: str, limit: int = 20):
    try:
        history = get_status_history(device_id, limit)
        logger.info("STATUS HIST  | device_id=%s | count=%s", device_id, len(history))

        return {
            "device_id": device_id,
            "storage": "mariadb",
            "count": len(history),
            "history": history,
        }
    except Exception as exc:
        logger.error("STATUS HIST  | failed device_id=%s | error=%s", device_id, exc)
        return {
            "device_id": device_id,
            "storage": "mariadb",
            "status": "failed",
            "error": str(exc),
        }



@app.get("/api/devices/{device_id}/status/latest")
def get_device_latest_status(device_id: str):
    try:
        status = get_latest_status(device_id)
        logger.info("STATUS GET   | device_id=%s | found=%s", device_id, bool(status))

        return {
            "device_id": device_id,
            "storage": "mariadb",
            "status": status,
        }
    except Exception as exc:
        logger.error("STATUS GET   | failed device_id=%s | error=%s", device_id, exc)
        return {
            "device_id": device_id,
            "storage": "mariadb",
            "status": None,
            "error": str(exc),
        }



@app.post("/api/devices/{device_id}/status")
def update_status(device_id: str, request: StatusRequest):
    logger.info(
        "STATUS IN    | device_id=%s | status=%s | message=%s | battery=%s | rssi=%s",
        device_id,
        request.status,
        request.message,
        request.battery,
        request.rssi,
    )

    try:
        save_status(
            device_id=device_id,
            status=request.status,
            message=request.message,
            battery=request.battery,
            rssi=request.rssi,
        )
        logger.info("DB STATUS    | saved device_id=%s | status=%s", device_id, request.status)
    except Exception as exc:
        logger.error("DB STATUS    | save failed device_id=%s | error=%s", device_id, exc)

    logger.info("STATUS SAVED | device_id=%s | storage=mariadb", device_id)

    return {
        "result": "status_saved",
        "device_id": device_id,
    }


@app.post("/api/devices/{device_id}/command/ack")
def acknowledge_command(device_id: str, request: AckRequest):
    logger.info(
        "ACK IN       | device_id=%s | msg_id=%s | result=%s | message=%s",
        device_id,
        request.msg_id,
        request.result,
        request.message,
    )

    # ACK storage is now handled by MariaDB.
    # RAM storage for acknowledgements is intentionally disabled.

    try:
        save_ack(
            msg_id=request.msg_id,
            device_id=device_id,
            result=request.result,
            message=request.message,
        )
        logger.info("DB ACK       | saved msg_id=%s | device_id=%s", request.msg_id, device_id)
    except Exception as exc:
        logger.error("DB ACK       | save failed msg_id=%s | error=%s", request.msg_id, exc)

    logger.info(
        "ACK SAVED    | device_id=%s | msg_id=%s | storage=mariadb",
        device_id,
        request.msg_id,
    )

    return {
        "result": "ack_saved",
        "device_id": device_id,
        "msg_id": request.msg_id,
    }


@app.get("/api/devices")
def get_devices():
    try:
        devices = list_devices()
        logger.info("DEVICES LIST | storage=mariadb | total_devices=%s", len(devices))

        return {
            "gateway_default": GATEWAY_NAME,
            "storage": "mariadb",
            "count": len(devices),
            "devices": devices,
        }
    except Exception as exc:
        logger.error("DEVICES LIST | failed | error=%s", exc)
        return {
            "storage": "mariadb",
            "status": "failed",
            "error": str(exc),
        }


@app.get("/api/debug/db")
def debug_db_state():
    try:
        state = get_db_debug_state()
        logger.info("DEBUG DB     | devices=%s | commands=%s | statuses=%s | acks=%s",
                    state["counts"].get("devices"),
                    state["counts"].get("commands"),
                    state["counts"].get("statuses"),
                    state["counts"].get("acks"))
        return state
    except Exception as exc:
        logger.error("DEBUG DB     | failed | error=%s", exc)
        return {
            "error": str(exc),
            "status": "failed",
        }


@app.get("/api/debug/state")
def debug_state():
    logger.info("DEBUG STATE  | ram_storage=disabled | use=/api/debug/db")

    return {
        "status": "ram_storage_disabled",
        "message": "Use /api/debug/db for MariaDB-backed debug state.",
        "db_debug_url": "/api/debug/db",
    }
