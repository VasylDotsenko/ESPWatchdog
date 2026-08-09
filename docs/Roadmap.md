# ESP Watchdog — Roadmap

Дата оновлення: 09.08.2026

Поточний baseline:

```text
0.4.40-web-api-auth
```

Production target:

```text
1.0.0
```

---

## Поточний стан

Проєкт вже має робочий runtime для ESP8266 / WeMos D1 mini:

- завантаження конфігурації з LittleFS;
- WiFi connection;
- TCP/SSH HealthCheck;
- Watchdog decision-layer;
- Tuya LAN `3.5` power-control для `TCOGCZ16-A`;
- restart history;
- Web API;
- lightweight Web Dashboard;
- Web configuration editor;
- controlled Tuya status polling policy;
- Web API token protection for state-changing commands.

Поточний етап — інтеграційна стабілізація перед production hardening.

---

## Завершено

### Core

- `Application`;
- `IService`;
- `Timer`;
- `Version.h`;
- `BuildInfo.h`;
- глобальні сервіси:
  - `Log`;
  - `Config`;
  - `Network`;
  - `System`;
  - `HealthCheck`;
  - `Watchdog`;
  - `Power`;
  - `TuyaLan`;
  - `WebServer`.

### Config

- `config.json`;
- `ConfigService`;
- LittleFS load/save;
- default config;
- validation;
- `GET /api/config`;
- `POST /api/config`;
- Web configuration editor.

### Logger / Storage

- printf-style logger;
- рівні логування;
- RAM/Flash string support;
- Storage синхронізовано з Logger;
- JSON read/write diagnostics.

### WiFi / SystemInfo

- `WiFiService`;
- `SystemInfoService`;
- `NetworkStatusData`;
- `SystemStatusData`;
- API-ready snapshots.

### HealthCheck

- `HealthCheckService`;
- `HealthCheckInfo`;
- `HealthCheckResult`;
- `IHealthCheckProvider`;
- `IcmpHealthCheckProvider`;
- native ESP8266 SDK ICMP session;
- TCP/SSH HealthCheck provider;
- API-ready `HealthStatusData`.

### Watchdog

- `WatchdogService`;
- restart decision-layer;
- cooldown через `bootDelay`;
- restart limit через `maxRestartPerDay`;
- API-ready `WatchdogStatusData`.

### Power / Tuya

- `IPowerController`;
- `PowerService`;
- `TuyaPowerController`;
- `TuyaService`;
- `TuyaCrypto`;
- `TuyaPacket`;
- `TuyaProtocol`;
- Tuya LAN `3.5` session negotiation;
- Tuya LAN `3.5` relay command path;
- manual power API commands:
  - `POST /api/power/on`;
  - `POST /api/power/off`;
  - `POST /api/power/restart`;
- dashboard power buttons.

### Web API / Dashboard

- `GET /`;
- `GET /api`;
- `GET /api/status`;
- `GET /api/system`;
- `GET /api/network`;
- `GET /api/health`;
- `GET /api/watchdog`;
- `GET /api/power`;
- `GET /api/config`;
- `POST /api/config`;
- `POST /api/power/on`;
- `POST /api/power/off`;
- `POST /api/power/restart`;
- dashboard status cards;
- dashboard command log;
- dashboard restart history;
- dashboard controlled host config;
- dashboard Tuya socket config;
- dashboard full config editor.

---

## Наступний етап — 0.4.x Stabilization

### 0.4.35 — Config pages + System restart API

Мета:

- винести налаштування кожної секції на окрему сторінку;
- додати безпечний reboot ESP з Web API;
- показувати після збереження конфігу повідомлення `Restart recommended`;
- додати кнопку `Restart ESP` на dashboard.

Planned endpoints:

```text
POST /api/system/restart
```

Статус: реалізовано.

### 0.4.36 — Runtime log buffer

Мета:

- додати ring-buffer для runtime logs;
- вивести logs у Web API;
- показати logs на dashboard.

Planned endpoints:

```text
GET /api/logs
```

Статус: реалізовано.

### 0.4.37 — First boot WiFi setup

Мета:

- якщо `wifi.ssid` порожній, автоматично підняти setup AP;
- якщо підключення до домашньої мережі не вдалося, перейти в setup AP;
- дозволити налаштувати WiFi через Web UI;
- не запускати Watchdog/Power-cycle у setup mode.

Setup AP:

```text
SSID: ESP-Watchdog-Setup
Password: 12345678
URL: http://192.168.4.1/config/wifi
```

Статус: реалізовано.

### 0.4.38 — Tuya status polling policy

Мета:

- додати явні налаштування `tuya.statusPollingEnabled` і `tuya.statusPollingInterval`;
- стабілізувати Tuya status polling;
- не провокувати disconnect на Tuya LAN `3.5`;
- не виконувати автоматичний `3.5` status query до реалізації окремого `6699 DPQuery`;
- показувати реальний стан relay на dashboard.

Статус: реалізовано як safe opt-in policy.

### 0.4.39 — TCP/SSH HealthCheck

Мета:

- замінити нестабільний ICMP Ping на TCP connect перевірку;
- використовувати `watchdog.targetPort`;
- для SSH-based health check використовувати port `22`;
- залишити ICMP provider як fallback.

Статус: реалізовано.

### 0.4.40 — Web API authentication

Мета:

- захистити state-changing endpoints;
- додати API token у `X-API-Token`;
- додати fallback `?token=...` для ручного тестування;
- не дозволяти випадковий power-cycle без авторизації.

Critical endpoints:

```text
POST /api/config
POST /api/power/on
POST /api/power/off
POST /api/power/restart
POST /api/system/restart
```

Статус: реалізовано.

### 0.4.41 — Dashboard polish

Мета:

- покращити UX;
- рознести Dashboard / Config / Logs;
- додати зрозуміші статуси помилок;
- додати reconnect/reload feedback.

---

## Production hardening — 0.5.x

### 0.5.0 — Security baseline

- harden Web API authentication;
- safe defaults;
- не показувати секрети;
- document threat model для LAN-only deployment.

### 0.5.1 — Config apply policy

- визначити, які налаштування застосовуються live;
- визначити, які потребують reboot;
- додати `requiresRestart` у `POST /api/config` response.

### 0.5.2 — Diagnostics

- heap diagnostics;
- uptime diagnostics;
- Tuya session diagnostics;
- watchdog reason diagnostics;
- export status snapshot.

### 0.5.3 — Recovery behavior

- покращення поведінки при:
  - WiFi reconnect;
  - Tuya unavailable;
  - config invalid;
  - LittleFS write failure;
  - repeated watchdog failures.

---

## Release Candidate — 0.9.x

### 0.9.0 — Feature freeze

- заборона великих архітектурних змін;
- тільки bugfix/stability;
- оновлення документації;
- перевірка повної збірки.

### 0.9.1 — Hardware verification

Перевірити на реальному hardware:

- cold boot;
- WiFi reconnect;
- HealthCheck online/offline;
- Watchdog trigger;
- Tuya power OFF;
- Tuya power ON;
- restart history;
- Web config save;
- reboot after config change.

### 0.9.2 — Long run test

- 24h runtime;
- memory stability;
- no heap fragmentation symptoms;
- no unexpected WDT reset;
- Tuya LAN stability.

---

## Production release — 1.0.0

Критерії готовності:

- firmware стабільно стартує;
- config створюється/читається/зберігається;
- WiFi reconnect працює;
- HealthCheck стабільний;
- Watchdog не створює restart-loop;
- Tuya LAN power-cycle hardware-verified;
- Web API захищений;
- dashboard показує runtime state;
- dashboard дозволяє змінювати config;
- logs/diagnostics доступні;
- документація актуальна.

---

## Після 1.0

Можливі напрями:

- MQTT status publishing;
- HTTP/TCP HealthCheck providers;
- OTA update;
- Web UI assets у LittleFS;
- backup/restore config;
- multiple controlled hosts;
- multiple Tuya sockets;
- ESP32 portability layer.
