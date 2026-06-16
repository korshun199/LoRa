from sqlalchemy import text
from app.db.database import engine


def save_device(device_id: str, device_type: str = "gateway", firmware_version: str | None = None) -> None:
    sql = text("""
        INSERT INTO devices (device_id, device_type, firmware_version, status)
        VALUES (:device_id, :device_type, :firmware_version, 'registered')
        ON DUPLICATE KEY UPDATE
            device_type = VALUES(device_type),
            firmware_version = VALUES(firmware_version),
            status = 'registered'
    """)

    with engine.begin() as connection:
        connection.execute(sql, {
            "device_id": device_id,
            "device_type": device_type,
            "firmware_version": firmware_version,
        })


def save_command(
    msg_id: str,
    target_id: str,
    command: str,
    value_text: str | None = None,
    channel: int | None = None,
) -> None:
    sql = text("""
        INSERT INTO commands (msg_id, target_id, command, value_text, channel, status)
        VALUES (:msg_id, :target_id, :command, :value_text, :channel, 'pending')
    """)

    with engine.begin() as connection:
        connection.execute(sql, {
            "msg_id": msg_id,
            "target_id": target_id,
            "command": command,
            "value_text": value_text,
            "channel": channel,
        })


def save_ack(msg_id: str, device_id: str, result: str, message: str | None = None) -> None:
    insert_ack = text("""
        INSERT INTO acks (msg_id, device_id, result, message)
        VALUES (:msg_id, :device_id, :result, :message)
    """)

    update_command = text("""
        UPDATE commands
        SET status = 'acknowledged',
            ack_result = :result
        WHERE msg_id = :msg_id
    """)

    with engine.begin() as connection:
        connection.execute(insert_ack, {
            "msg_id": msg_id,
            "device_id": device_id,
            "result": result,
            "message": message,
        })
        connection.execute(update_command, {
            "msg_id": msg_id,
            "result": result,
        })
