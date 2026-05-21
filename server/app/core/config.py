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
