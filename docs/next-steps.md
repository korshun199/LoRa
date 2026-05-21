# LoRa Next Steps

## Ближайший порядок работ

1. Подключить MariaDB к локальному FastAPI dev-серверу.
2. Создать таблицы devices, commands, statuses, acks.
3. Перенести in-memory хранение в MariaDB.
4. Подготовить deploy-процесс на VPS через Git.
5. Сделать gateway firmware: Wi-Fi setup portal + связь с VPS.
6. Сделать remote node firmware: LoRa receiver + управление GPIO.
7. Добавить LoRa-обмен gateway <-> node.

## Текущий принцип

Ноутбук = dev-среда.
GitHub = хранилище кода.
VPS = будущий боевой сервер.
Gateway = мост VPS <-> LoRa-сеть.
Remote Node = удалённое оборудование вне Wi-Fi.
