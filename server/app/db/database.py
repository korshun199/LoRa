import os
from sqlalchemy import create_engine, text
from app.core.config import DB_HOST, DB_PORT, DB_NAME, DB_USER

DB_PASSWORD = os.getenv("DB_PASSWORD", "change_me_later")

DATABASE_URL = f"mysql+pymysql://{DB_USER}:{DB_PASSWORD}@{DB_HOST}:{DB_PORT}/{DB_NAME}?charset=utf8mb4"

engine = create_engine(DATABASE_URL, pool_pre_ping=True)

def check_database_connection() -> tuple[bool, str]:
    try:
        with engine.connect() as connection:
            version = connection.execute(text("SELECT VERSION()")).scalar()
        return True, str(version)
    except Exception as exc:
        return False, str(exc)
