# ESP Watchdog

ESP Watchdog — автономний мережевий watchdog на базі ESP8266, призначений для автоматичного відновлення роботи серверів, Home Assistant та іншого мережевого обладнання шляхом циклічного контролю доступності та дистанційного перезапуску живлення через Tuya Wi-Fi Smart Plug.

Проєкт розробляється як повністю незалежний пристрій і не потребує Home Assistant, MQTT або будь-якого зовнішнього сервера для роботи.

---

## Поточний статус

**Версія:** `v0.4.5-tuya-packet`

Статус:

> 🚧 Active Development

Основний напрямок розробки:

- Tuya LAN Protocol
- Tuya AES Encryption
- Packet Layer
- Power Controller
- Web Configuration
- OTA Update

---

# Основні можливості

## Реалізовано

- модульна сервісна архітектура;
- неблокуючий runtime;
- централізований Logger;
- файлова система LittleFS;
- JSON-конфігурація;
- Wi-Fi Service;
- System Information Service;
- власний Timer;
- власний ICMP Ping стек;
- Health Check Service;
- Watchdog Decision Engine;
- захист від циклічних перезапусків;
- Dependency Injection для Health Providers.

---

## У розробці

- Tuya LAN Protocol
- Tuya Packet Layer
- Tuya Crypto (AES)
- Smart Plug Controller
- Web UI
- OTA Update
- Event Log

---

# Архітектура

```
Application
│
├── Core
│   ├── IService
│   ├── Timer
│   └── Logger
│
├── Models
│
├── Storage
│
├── Config
│
├── Network
│   ├── IcmpSession
│   ├── NetworkResult
│   └── NetworkTypes
│
├── Services
│   ├── WiFiService
│   ├── SystemInfoService
│   ├── HealthCheckService
│   └── WatchdogService
│
└── Tuya (WIP)
    ├── TuyaPacket
    ├── TuyaCrypto
    ├── TuyaProtocol
    └── TuyaService
```

---

# Алгоритм роботи

```
              Ping
                 │
                 ▼
        HealthCheck Service
                 │
         Server available?
          │            │
         Yes           No
          │             │
          │      Retry policy
          │             │
          │      Restart required?
          │             │
          │            Yes
          │             │
          ▼             ▼
      Continue     Tuya Power OFF
                        │
                    Boot Delay
                        │
                  Tuya Power ON
                        │
                 Continue monitoring
```

---

# Використані технології

- ESP8266
- Arduino Framework
- PlatformIO
- LittleFS
- ArduinoJson
- lwIP Native Ping API

---

# Структура проєкту

```
src/
│
├── Core/
├── Models/
├── Network/
├── Services/
├── Storage/
└── main.cpp

docs/
│
├── Architecture.md
├── CodingStyle.md
├── Roadmap.md
├── ProjectStatus.md
└── Changelog.md
```

---

# Конфігурація

Конфігурація зберігається у LittleFS.

Основні параметри:

- Wi-Fi
- Hostname
- Target IP
- Ping Interval
- Retry Count
- Restart Delay
- Maximum Restarts Per Day
- Tuya Device ID
- Local Key

---

# Roadmap

## v0.5

- Tuya Packet
- AES Encryption

## v0.6

- Tuya Protocol

## v0.7

- Tuya Service

## v0.8

- Web Configuration

## v0.9

- OTA Update

## v1.0

Перша production-ready версія.

---

# Документація

Проєкт містить окрему технічну документацію:

- Architecture.md
- CodingStyle.md
- Roadmap.md
- ProjectStatus.md
- Changelog.md

---

# Ліцензія

MIT License

---

# Автор

**Vasyl Dotsenko**