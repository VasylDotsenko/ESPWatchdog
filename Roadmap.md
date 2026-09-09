# ESP Watchdog — Roadmap

Дата оновлення: 08.09.2026

Поточний baseline:

```text
0.9.2-long-run-test
```

Production target:

```text
1.0.0
```

---

## Поточний статус

ESP Watchdog вже має робочий runtime для ESP8266 / WeMos D1 mini:

- завантаження конфігурації з LittleFS;
- first-boot WiFi setup portal;
- WiFi AP recovery після втрати домашньої мережі;
- TCP/SSH HealthCheck;
- Watchdog decision-layer;
- Tuya LAN `3.5` power-control для розетки `TCOGCZ16-A`;
- restart history;
- Web API;
- Web Dashboard;
- Web configuration editor;
- runtime logs page;
- API token protection для state-changing commands;
- mobile-friendly Web UI для dashboard/config/logs;
- route fallback для Web UI config pages;
- Web API security baseline;
- diagnostics baseline;
- runtime recovery guard;
- OTA update over WiFi.

Поточний етап — **Release Candidate `0.9.x` / Long-run Test**.

---

## Завершено у 0.4.x

### Core

- `Application`;
- `IService`;
- `Timer`;
- `Version.h`;
- `BuildInfo.h`;
- глобальні сервіси:
  - `Log`;
  - `Storage`;
  - `Config`;
  - `Network`;
  - `System`;
  - `TuyaLan`;
  - `Power`;
  - `HealthCheck`;
  - `Watchdog`;
  - `WebServer`.

### Storage / Config

- LittleFS mount;
- `config.json` load/save;
- default configuration;
- validation;
- `ConfigService::updateFromJson(...)`;
- `GET /api/config`;
- `POST /api/config`;
- streaming `GET /api/config`;
- Web configuration editor;
- окремі сторінки налаштувань:
  - `/config/device`;
  - `/config/wifi`;
  - `/config/watchdog`;
  - `/config/relay`;
  - `/config/tuya`;
  - `/config/security`.

### Logger

- printf-style logger;
- рівні логування;
- підтримка RAM-рядків і Flash-рядків;
- runtime ring-buffer;
- `GET /api/logs`;
- окрема сторінка `/logs`;
- streaming JSON для `/api/logs`;
- виправлено stack-heavy log serialization.

### WiFi / SystemInfo

- `WiFiService`;
- first-boot setup AP:

```text
SSID: ESP-Watchdog-Setup
Password: 12345678
URL: http://192.168.4.1/config/wifi
```

- setup mode без запуску Watchdog/Power-cycle;
- `SystemInfoService`;
- `NetworkStatusData`;
- `SystemStatusData`;
- API-ready snapshots.

### HealthCheck

- `HealthCheckService`;
- `HealthCheckResult`;
- `HealthCheckInfo`;
- `IHealthCheckProvider`;
- `IcmpHealthCheckProvider`;
- native ESP8266 SDK ICMP session;
- `TcpHealthCheckProvider`;
- активний production HealthCheck через TCP connect;
- SSH-based перевірка через `watchdog.targetPort = 22`;
- ICMP залишено як fallback/diagnostic provider.

### Watchdog

- `WatchdogService`;
- restart decision-layer;
- `bootDelay` cooldown;
- `maxRestartPerDay` limit;
- restart completion callback;
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
- Tuya LAN `3.5` relay OFF/ON command path;
- command ACK handling;
- on-demand Tuya connect перед командою;
- Tuya status polling policy:
  - `tuya.statusPollingEnabled`;
  - `tuya.statusPollingInterval`;
  - polling disabled by default;
- manual power API commands:
  - `POST /api/power/on`;
  - `POST /api/power/off`;
  - `POST /api/power/restart`;
- dashboard power buttons;
- restart history.

### Web API / Dashboard

- `GET /`;
- `GET /api`;
- `GET /api/status`;
- subsystem endpoints:
  - `GET /api/system`;
  - `GET /api/network`;
  - `GET /api/health`;
  - `GET /api/watchdog`;
  - `GET /api/power`;
- command endpoints:
  - `POST /api/system/restart`;
  - `POST /api/config`;
  - `POST /api/power/on`;
  - `POST /api/power/off`;
  - `POST /api/power/restart`;
- API token protection:
  - `security.apiAuthEnabled`;
  - `security.apiToken`;
  - `Authorization: Bearer <token>`;
  - no query-token fallback;
- Web Dashboard lightweight mode;
- config pages no longer call heavy `/api/status`;
- dashboard can use aggregate `/api/status` as a fast path and falls back to subsystem endpoints;
- Web UI split:
  - `WebPages.h`;
  - `WebPages.cpp`;
- JSON helpers split:
  - `WebJsonUtils.h`;
  - `WebJsonUtils.cpp`;
- config API split:
  - `WebApiConfig.h`;
  - `WebApiConfig.cpp`.

---

## Поточний етап — 0.4.x WebServer split

Мета: зробити `WebServerService.cpp` читабельним, стабільним і дешевим по RAM/stack.

### 0.4.50 — WebApiLogs split

Статус: реалізовано.

План:

- винести `GET /api/logs` з `WebServerService.cpp`;
- створити:
  - `WebApiLogs.h`;
  - `WebApiLogs.cpp`;
- залишити streaming JSON;
- не змінювати поведінку `/logs`.

### 0.4.51 — WebApiPower split

Статус: реалізовано.

План:

- винести manual power endpoints:
  - `POST /api/power/on`;
  - `POST /api/power/off`;
  - `POST /api/power/restart`;
- залишити authorization у `WebServerService`;
- централізувати JSON responses для command handlers.

### 0.4.52 — ESP restart safe

Статус: реалізовано.

План:

- стабілізувати `POST /api/system/restart`;
- дати браузеру отримати HTTP-відповідь до фактичного restart;
- уникнути додаткового логування в момент безпосереднього `ESP.restart()`;
- зменшити ризик `Exception (29)` / `ctx: sys` після натискання кнопки Restart ESP.

### 0.4.53 — Dashboard resilient loading

Статус: реалізовано.

План:

- додати route `/dashboard`;
- зробити `/api/config` optional для головної сторінки;
- дозволити Dashboard відкриватися при частковій недоступності API;
- показувати конкретну помилку endpoint-а замість загального `unable to load`.

### 0.4.54 — Dashboard fast load

Статус: реалізовано.

План:

- використовувати `/api/status` як fast path для Dashboard;
- залишити split endpoints як fallback;
- не блокувати перший render через `/api/config`;
- кешувати конфігурацію для Dashboard;
- зменшити частоту auto-refresh;
- прибрати накладання одночасних browser fetch циклів.

### 0.4.55 — WebApiStatus split

Статус: реалізовано.

План:

- винести read-only subsystem endpoints:
  - `/api/system`;
  - `/api/network`;
  - `/api/health`;
  - `/api/watchdog`;
  - `/api/power`;
- оцінити потребу у streaming serializer для status endpoints;
- залишити `/api/status` як compatibility endpoint.

### 0.4.56 — WebServer route cleanup

Статус: реалізовано.

План:

- зробити route registration компактнішим;
- згрупувати routes:
  - pages;
  - read-only API;
  - config API;
  - command API;
  - CORS/OPTIONS;
- зменшити `WebServerService.cpp` до ролі coordinator-а.

### 0.4.57 — Web command/auth cleanup

Статус: реалізовано.

План:

- винести command authorization helper-и з `WebServerService.cpp`;
- централізувати CORS/JSON response helpers для Web API modules;
- прибрати дублювання `sendJson()` між `WebServerService`, `WebApiStatus` і `WebApiIndex`;
- підготувати базу для security hardening у `0.5.x`.

### 0.4.58 — Web UI production polish

Статус: реалізовано.

План:

- покращити UX config pages;
- додати зрозумілі повідомлення після save config;
- показувати `restartRecommended`;
- підготувати UI до `requiresRestart` / live apply policy;
- перевірити mobile layout;
- мінімізувати HTML/JS без втрати читабельності.

### 0.4.59 — Config apply policy

Статус: реалізовано.

План:

- визначити, які секції config застосовуються live;
- визначити, які секції потребують restart ESP;
- додати `requiresRestart` у відповідь `POST /api/config`;
- додати section-level apply policy для:
  - device;
  - wifi;
  - watchdog;
  - relay;
  - tuya;
  - security;
- синхронізувати Web UI з цією політикою.

### 0.4.60 — Web UI mobile polish

Статус: реалізовано.

Реалізовано:

- покращити spacing для малих екранів;
- зробити action buttons зручнішими на touch UI;
- зменшити візуальний шум у footer API links;
- перевірити довгі Tuya/device values на overflow.
- приховати API endpoint links у collapsible footer;
- збільшити `input` / `button` touch targets;
- оптимізувати logs view для mobile.

---

## Production hardening — 0.5.x

### 0.5.0 — Security baseline

Статус: реалізовано.

- harden Web API authentication;
- safe defaults;
- не показувати секрети у відкритому вигляді;
- перевірити behavior при порожньому `security.apiToken`;
- LAN-only threat model;
- audit state-changing endpoints.

### 0.5.1 — Diagnostics baseline

Статус: реалізовано.

- heap diagnostics;
- uptime diagnostics;
- reset reason diagnostics;
- WiFi reconnect diagnostics;
- Tuya session diagnostics;
- watchdog reason diagnostics;
- export runtime snapshot.

### 0.5.2 — Recovery behavior

Статус: реалізовано частково.

- WiFi reconnect behavior;
- Tuya unavailable behavior;
- config invalid behavior;
- LittleFS write failure behavior;
- repeated watchdog failures behavior;
- restart-loop protection review.
- RuntimeGuard для degraded heap / heap fragmentation.

### 0.5.3 — OTA update

Статус: реалізовано.

- ArduinoOTA service;
- OTA upload через PlatformIO `espota`;
- OTA hostname з config;
- OTA password через `security.apiToken`;
- RuntimeGuard pause під час OTA upload.

### 0.5.4 — Memory / stack audit

Статус: реалізовано частково.

- audit `JsonDocument` usage;
- audit stack buffers;
- перевірити WebServer handlers під навантаженням;
- прибрати зайві великі локальні буфери;
- перевірити 24h heap stability.
- `/api/diagnostics` переведено на streaming JSON без runtime `JsonDocument`;
- постійний WebServer JSON buffer зменшено з 8192 до 6144 bytes;
- `SystemInfo` більше не оновлює firmware/reset strings щосекунди.
- runtime log buffer зменшено до 16 entries;
- RuntimeGuard diagnostics синхронізовано з поточним heap.

### 0.5.5 — Real-time logs

Статус: реалізовано.

- додано `TimeService`;
- NTP sync після підключення до WiFi;
- timezone для Europe/Kyiv;
- Serial logs після NTP показують реальний час події;
- `/api/logs` віддає `wallTime` та `wallTimeText`;
- Web logs page показує real-time timestamp, якщо час синхронізований.

### 0.5.6 — Log time backfill

Статус: реалізовано.

- стартові log entries отримують real-time timestamp після NTP sync;
- `Logger::synchronizeWallTime()` заповнює `wallTime` / `wallTimeText` для вже наявних записів;
- timezone налаштовується через ESP8266-native `configTime(TZ, ...)`;
- виправлено некоректне відображення стартового логу після синхронізації часу.

### 0.5.7 — Long-run observability

Статус: реалізовано.

- додано heap trend metrics у `RuntimeGuardStatus`;
- `/api/diagnostics` показує:
  - `heapAtBoot`;
  - `minFreeHeapSeen`;
  - `heapDropFromBoot`;
  - `maxHeapFragmentationSeen`;
  - `lastCheckAt`.
- dashboard отримав картку `Runtime trend`;
- спрощено контроль 24h / 7d stability без ручного порівняння JSON-зрізів.

### 0.5.8 — Tuya runtime observability

Статус: реалізовано.

- додано Tuya runtime timestamps:
  - `connectedAt`;
  - `lastDisconnectedAt`;
  - `lastCommandAt`;
  - `lastPacketAt`;
  - `lastErrorAt`.
- додано лічильники Tuya `3.5` session negotiation:
  - `sessionStartCount`;
  - `sessionEstablishedCount`;
  - `sessionFailureCount`.
- `/api/diagnostics` отримав секцію `tuya`;
- dashboard отримав картку `Tuya runtime`;
- Tuya RX path отримав додаткові `yield()` під час читання пакетів.

### 0.5.9 — RC readiness

Статус: реалізовано.

- поточний baseline синхронізовано з фактичним станом прошивки;
- README більше не позначає security baseline та OTA як незавершені;
- зафіксовано реальні перед-RC blockers:
  - Tuya LAN `3.5` status DPQuery через `6699`;
  - long-run verification після останніх `0.5.x` змін;
  - фінальна перевірка документації / baseline config перед `0.9.0`.
- наступний великий milestone визначено як `0.9.0 Feature freeze`.

### 0.5.10 — Tuya 3.5 status DPQuery

Статус: реалізовано, потребує hardware verification.

- додано `Tuya::Protocol::buildStatusQuery(..., Packet6699&)`;
- для Tuya LAN `3.5` status query тепер іде через:
  - `ensureSession35()`;
  - command `DPQueryNew`;
  - encrypted `6699` packet;
  - relay DPS з `config.json`.
- `statusPollingEnabled` тепер може запускати polling і для протоколу `3.5`;
- relay command path залишено без функціональних змін.

---

## Release Candidate — 0.9.x

### 0.9.0 — Feature freeze

Статус: реалізовано.

- заборона великих архітектурних змін;
- тільки bugfix/stability;
- документація актуалізована;
- повна збірка без warnings/error;
- baseline config.json перевірений.
- додано `ReleaseChecklist.md`;
- production feature set зафіксовано для `1.0.0`;
- нові функції переносяться після `1.0.0`, якщо вони не блокують hardware verification.

### 0.9.1 — Hardware verification

Статус: реалізовано / триває польова перевірка.

Підготовлено:

- `HardwareVerification.md`;
- покрокові browser/API/Serial сценарії;
- expected logs;
- pass/fail критерії.

Перевірити на реальному hardware:

- cold boot;
- first boot setup portal;
- WiFi reconnect;
- TCP/SSH HealthCheck online/offline;
- Watchdog trigger;
- Tuya power OFF;
- Tuya power ON;
- restart history;
- Web config save;
- ESP restart з Web UI;
- reboot after config change.

### WiFi AP recovery hotfix

Статус: реалізовано.

Причина:

- під час unattended роботи, якщо домашня WiFi-мережа зникала, пристрій міг залишатися в `ESP-Watchdog-Setup` AP mode надто довго;
- для production watchdog це небажано, бо пристрій має самостійно відновлювати STA-підключення або перезапускатися.

Реалізовано:

- setup portal rescue window — 5 хвилин;
- після 5 хвилин AP вимикається і пристрій повторно пробує підключитися до налаштованої WiFi-мережі;
- якщо WiFi не відновився протягом 20 хвилин, ESP виконує контрольований restart;
- first-boot сценарій із порожнім `wifi.ssid` не перезапускає ESP і залишається доступним для первинного налаштування.

### 0.9.2 — Long run test

Основні документи етапу:

- `LongRunTest.md`;
- `LongRunReport.md`.

- 24h runtime;
- memory stability;
- no heap fragmentation symptoms;
- no unexpected WDT reset;
- Tuya LAN stability;
- Web UI stability;
- no restart loop.

---

## Production release — 1.0.0

Критерії готовності:

- firmware стабільно стартує;
- config створюється/читається/зберігається;
- WiFi reconnect працює;
- first boot setup працює;
- AP rescue mode не зависає назавжди після втрати домашньої WiFi-мережі;
- TCP/SSH HealthCheck стабільний;
- Watchdog не створює restart-loop;
- Tuya LAN power-cycle hardware-verified;
- restart history працює;
- Web API захищений;
- dashboard показує runtime state;
- dashboard дозволяє змінювати config;
- logs/diagnostics доступні;
- документація актуальна;
- 24h runtime test пройдено.

---

## Після 1.0

Можливі напрями:

- OTA update;
- MQTT status publishing;
- HTTP HealthCheck provider;
- HTTPS HealthCheck provider;
- Web UI assets у LittleFS;
- backup/restore config;
- multiple controlled hosts;
- multiple Tuya sockets;
- ESP32 portability layer.
