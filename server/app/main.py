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
