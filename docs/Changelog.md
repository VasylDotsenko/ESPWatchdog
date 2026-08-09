# ESP Watchdog

> Журнал змін проєкту

---

## [0.4.24-network-status-snapshot] - 09.08.2026

### Статус

Додано API-ready snapshot модель для Network/WiFi subsystem.

Це п'ятий базовий snapshot-шар. Тепер усі ключові runtime subsystem-и мають стабільні POD-знімки для майбутнього Web API.

### Оновлено

- `Models/NetworkStatusData.h`;
- `Models/NetworkData.h`;
- `Services/WiFi/WiFiService.h`;
- `Services/WiFi/WiFiService.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`;
- `Changelog.md`.

### Додано

- `NetworkStatusSummary`;
- `NetworkStatusConfiguration`;
- `NetworkStatusAddress`;
- `NetworkStatusSignal`;
- `NetworkStatusStatistics`;
- `NetworkStatusData`;
- `WiFiService::status()`.

### Містить

- WiFi connection state;
- hostname;
- SSID;
- IP/gateway/subnet/DNS;
- RSSI;
- signal quality;
- reconnect statistics;
- connect/disconnect timestamps.

### Призначення

`NetworkStatusData` стане джерелом для:

- `/api/network/status`;
- `/api/status`;
- diagnostics endpoint;
- Web Dashboard;
- майбутнього MQTT telemetry.

### Версія

```text
0.4.24-network-status-snapshot
```

---

## [0.4.23-system-status-snapshot] - 09.08.2026

### Статус

Додано API-ready snapshot модель для System subsystem.

Це четвертий базовий snapshot-шар після `PowerStatusData`, `HealthStatusData` і `WatchdogStatusData`. Тепер майбутній Web API зможе показати firmware/build/runtime/system diagnostics без прямого доступу до внутрішнього `SystemData`.

### Оновлено

- `Models/SystemStatusData.h`;
- `Models/SystemData.h`;
- `Services/SystemInfo/SystemInfo.h`;
- `Services/SystemInfo/SystemInfo.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`;
- `Changelog.md`.

### Додано

- `SystemStatusFirmware`;
- `SystemStatusUptime`;
- `SystemStatusMemory`;
- `SystemStatusCpu`;
- `SystemStatusData`;
- `SystemInfoService::status()`.

### Містить

- firmware version;
- SDK/Core version;
- build date/time;
- reset reason;
- uptime;
- heap/flash/sketch statistics;
- chip id;
- CPU frequency.

### Призначення

`SystemStatusData` стане джерелом для:

- `/api/system/status`;
- `/api/status`;
- diagnostics endpoint;
- Web Dashboard;
- майбутнього MQTT telemetry.

### Версія

```text
0.4.23-system-status-snapshot
```

---

## [0.4.22-watchdog-status-snapshot] - 09.08.2026

### Статус

Додано API-ready snapshot модель для Watchdog subsystem.

Це третій snapshot-шар після `PowerStatusData` і `HealthStatusData`. Тепер майбутній Web API зможе показати не тільки стан target host і power controller, а й саме рішення watchdog: monitoring, restart required, cooldown або locked out.

### Оновлено

- `Models/WatchdogStatusData.h`;
- `Services/Watchdog/WatchdogService.h`;
- `Services/Watchdog/WatchdogService.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`;
- `Changelog.md`.

### Додано

- `WatchdogStatusSummary`;
- `WatchdogStatusConfiguration`;
- `WatchdogStatusStatistics`;
- `WatchdogStatusData`;
- `WatchdogService::status()`.

### Містить

- поточний `WatchdogState`;
- `enabled`;
- `restartPending`;
- `restartRequired`;
- `lockedOut`;
- `cooldown`;
- `consecutiveFailures`;
- watchdog configuration snapshot;
- restart statistics;
- timestamps останнього success/failure/restart/lockout.

### Призначення

`WatchdogStatusData` стане джерелом для:

- `/api/watchdog/status`;
- diagnostics endpoint;
- Web Dashboard;
- майбутнього MQTT telemetry.

### Версія

```text
0.4.22-watchdog-status-snapshot
```

---

## [0.4.21-health-status-snapshot] - 09.08.2026

### Статус

Додано API-ready snapshot модель для HealthCheck subsystem.

Це другий підготовчий етап перед Web API після `PowerStatusData`: `HealthCheckService` тепер може віддати стабільний POD-знімок стану перевірки доступності без прив'язки до JSON, Logger або WebServer.

### Оновлено

- `Models/HealthStatusData.h`;
- `Services/HealthCheck/HealthCheckService.h`;
- `Services/HealthCheck/HealthCheckService.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`;
- `Changelog.md`.

### Додано

- `HealthStatusSummary`;
- `HealthStatusStatistics`;
- `HealthStatusTimestamps`;
- `HealthStatusData`;
- `HealthCheckService::status()`.

### Містить

- поточну доступність target host;
- стан виконання health-check;
- останній `HealthCheckStatus`;
- останній RTT;
- статистику sent/received/lost;
- consecutive success/fails;
- min/max RTT;
- timestamps останньої перевірки, success/fail і зміни availability.

### Призначення

`HealthStatusData` стане джерелом для:

- `/api/health/status`;
- diagnostics endpoint;
- Web Dashboard;
- майбутнього MQTT telemetry.

### Версія

```text
0.4.21-health-status-snapshot
```

---

## [0.4.20-power-status-snapshot] - 09.08.2026

### Статус

Додано API-ready snapshot модель для Power subsystem.

Це підготовчий етап перед Web API: `PowerService` тепер може віддати повний, стабільний POD-знімок свого стану без прив'язки до JSON, WebServer або Logger.

### Оновлено

- `Models/PowerStatusData.h`;
- `Services/Power/PowerService.h`;
- `Services/Power/PowerService.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`;
- `Changelog.md`.

### Додано

- `PowerStatusSummary`;
- `PowerStatusStatistics`;
- `RestartHistoryStatus`;
- `PowerStatusData`;
- `PowerService::status()`.

### Містить

- поточний `PowerState`;
- доступність power-controller;
- restart runtime flags;
- активний restart id;
- power statistics;
- restart history counters;
- копію ring-buffer restart history entries.

### Призначення

`PowerStatusData` стане джерелом для:

- `/api/power/status`;
- `/api/power/history`;
- diagnostics endpoint;
- майбутнього MQTT telemetry;
- Web Dashboard.

### Версія

```text
0.4.20-power-status-snapshot
```

---

## [0.4.19-restart-history-log] - 09.08.2026

### Статус

Додано runtime-логування restart history entries.

Це проміжний observability-крок перед Web/API export: після кожного успішного або невдалого power-cycle firmware одразу друкує короткий запис історії в Serial log.

### Оновлено

- `Services/Power/PowerService.h`;
- `Services/Power/PowerService.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`;
- `Changelog.md`.

### Додано

- текстове представлення `RestartResult`;
- текстове представлення `RestartReason`;
- `PowerService::logRestartHistoryEntry()`;
- компактний log рядок, сумісний із поточним `Logger::BUFFER_SIZE`.

### Очікуваний runtime log

```text
PowerService: restart completed
RestartHistory: id=1 success reason=watchdog_failure off=10000 ms dur=10002 ms
```

Для помилки:

```text
RestartHistory: id=2 failed reason=power_on_timeout off=10000 ms dur=60000 ms
```

### Версія

```text
0.4.19-restart-history-log
```

---

## [0.4.18-restart-history] - 09.08.2026

### Статус

Додано базову in-memory історію restart/power-cycle подій.

Після hardware-verified Tuya LAN `3.5` power-cycle наступним production-кроком стало збереження фактичних результатів перезапуску: коли restart почався, коли живлення було вимкнено, коли увімкнено, чи завершився цикл успішно, або чому він впав.

### Оновлено

- `Models/RestartHistoryData.h`;
- `Models/PowerData.h`;
- `Services/Power/PowerService.h`;
- `Services/Power/PowerService.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`;
- `Changelog.md`.

### Додано

- `RestartHistoryData`;
- `RestartHistoryEntry`;
- `RestartResult`;
- `RestartReason`;
- ring-buffer на останні 10 restart-подій;
- лічильники `total`, `succeeded`, `failed`;
- timestamps:
  - `startedAt`;
  - `completedAt`;
  - `powerOffAt`;
  - `powerOnAt`;
  - `lastStartedAt`;
  - `lastCompletedAt`;
  - `lastFailedAt`;
- `requestedPowerOffTime`;
- `actualDuration`;
- `controllerAvailableAtStart`.

### Змінено

- `PowerService::restart()` створює запис історії при старті restart-attempt;
- успішний `PowerService::powerOn()` завершує активний запис як `Success`;
- помилки `controller unavailable`, `powerOff failed`, `powerOn failed`, `powerOn timeout` пишуться в історію як `Failed`;
- `PowerData` тепер містить `restartHistory`.

### Призначення

Ця модель стане основою для:

- Web API `/power/history`;
- diagnostics endpoint;
- restart dashboard;
- аналізу failed restart attempts;
- майбутнього збереження історії у Flash.

### Версія

```text
0.4.18-restart-history
```

---

## [0.4.17-tuya-on-demand-idle] - 09.08.2026

### Статус

Стабілізовано idle-поведінку Tuya LAN service після успішного hardware-verified power-cycle.

Після виконання команди розетка може сама закривати TCP socket. Для Tuya LAN power-control це нормальна поведінка, тому `TuyaService` переведено в чистий on-demand режим.

### Оновлено

- `Services/Tuya/TuyaService.h`;
- `Services/Tuya/TuyaService.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`;
- `Changelog.md`.

### Змінено

- `TuyaService` більше не виконує background reconnect у `loop()`;
- підключення до Tuya socket виконується перед реальною командою;
- `queryStatus()` також підключається on-demand;
- idle disconnect більше не запускає автоматичний reconnect timer;
- runtime лог після power-cycle має бути чистішим і без зайвих reconnect/disconnect циклів.

### Очікувана поведінка

```text
Watchdog: restart required
Connecting to Tuya ...
Tuya connected
Tuya: 3.5 session established
Tuya: 3.5 relay command sent, state=0
Tuya: relay state=0
Tuya: 3.5 relay command sent, state=1
Tuya: relay state=1
PowerService: restart completed
```

Після цього socket може залишитись відкритим або бути закритим розеткою. `TuyaService` не буде перепідключатися без потреби.

### Версія

```text
0.4.17-tuya-on-demand-idle
```

---

## [0.4.16-tuya35-command-ack] - 09.08.2026

### Статус

Tuya LAN protocol `3.5` пройшов перший успішний hardware smoke-test із реальною розеткою.

Runtime flow підтвердив:

```text
Tuya: 3.5 session established
Tuya: 3.5 relay command sent, state=0
Tuya: relay state=0
Tuya: 3.5 relay command sent, state=1
Tuya: relay state=1
PowerService: restart completed
```

### Оновлено

- `Services/Tuya/TuyaService.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`;
- `Changelog.md`.

### Виправлено

- `CONTROL_NEW` відповідь `cmd=13` з коротким payload `4` більше не логується як помилка decrypt;
- такий пакет трактується як короткий command ACK;
- фактичний стан реле як і раніше береться із `STATUS cmd=8` / DPS payload;
- runtime лог став чистішим після успішної relay-команди.

### Примітка

Розетка може закривати TCP socket після idle-періоду. Це не є помилкою power-cycle, оскільки `TuyaService` працює у on-demand режимі й може перепідключатися перед наступною командою.

### Версія

```text
0.4.16-tuya35-command-ack
```

---

## [0.4.15-tuya35-session-retcode] - 09.08.2026

### Статус

Виправлено розбір `SESSION_KEY_RESP` для Tuya LAN protocol `3.5`.

За runtime-логом розетка вже відповідала на handshake:

```text
Tuya: 3.5 packet received, cmd=4 seq=11658 payload=52
Tuya: 3.5 session response invalid
```

`payload=52` означає, що після AES-GCM decrypt всередині відповіді є:

```text
retcode(4) + device_nonce(16) + HMAC(client_nonce)(32)
```

Попередня реалізація читала `device_nonce` з нульового байта payload і через це HMAC-перевірка завжди падала.

### Оновлено

- `Services/Tuya/TuyaProtocol.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`;
- `Changelog.md`.

### Виправлено

- `Protocol::processSessionResponse()` тепер пропускає encrypted retcode `0x00000000` у відповіді пристрою;
- `device_nonce` читається з правильного offset;
- HMAC перевіряється за правильним layout payload;
- Tuya 3.5 session negotiation має перейти до `SESSION_KEY_FINISH`.

### Очікуваний runtime flow

```text
Tuya: 3.5 session start sent, seq=1
Tuya: 3.5 packet received, cmd=4 seq=... payload=52
Tuya: 3.5 session established
Tuya: 3.5 relay command sent
```

### Версія

```text
0.4.15-tuya35-session-retcode
```

---

## [0.4.14-tuya35-wdt-safe] - 09.08.2026

### Статус

Виправлено runtime-падіння ESP8266 під час старту Tuya LAN protocol `3.5`.

За логом плата успішно підключалась до Tuya socket і відправляла `SESSION_KEY_START`, після чого отримувала `Soft WDT reset`.

### Оновлено

- `Services/Tuya/TuyaService.h`;
- `Services/Tuya/TuyaService.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`.

### Виправлено

- прибрано великі локальні `Packet6699` об'єкти зі stack у Tuya 3.5 command/session path;
- додано спільний `Packet6699` buffer як поле `TuyaService`;
- скорочено очікування відповіді Tuya 3.5 handshake до WDT-safe timeout;
- додано `yield()` під час очікування handshake-пакета;
- додано діагностичний лог timeout із кількістю отриманих байтів;
- при `disconnect()` скидається Tuya protocol/session state, щоб не використовувати старий session key після reconnect.

### Очікуваний runtime flow

```text
Tuya connected
Tuya: 3.5 session start sent, seq=1
Tuya: 3.5 packet received, cmd=4
Tuya: 3.5 session established
Tuya: 3.5 relay command sent
```

Якщо розетка не відповість на handshake:

```text
Tuya: 3.5 session response timeout, rx=0 connected=1
Tuya: 3.5 session negotiation failed
```

### Версія

```text
0.4.14-tuya35-wdt-safe
```

---

## [0.4.13-tuya35-gcm-frame] - 09.08.2026

### Статус

Розпочато повноцінну реалізацію Tuya LAN protocol `3.5`.

Цей етап додає новий транспортний кадр `6699`, AES-GCM crypto path, session-key negotiation і relay command path через `CONTROL_NEW`.

### Оновлено

- `Services/Tuya/TuyaCrypto.h`;
- `Services/Tuya/TuyaCrypto.cpp`;
- `Services/Tuya/TuyaPacket.h`;
- `Services/Tuya/TuyaPacket.cpp`;
- `Services/Tuya/TuyaProtocol.h`;
- `Services/Tuya/TuyaProtocol.cpp`;
- `Services/Tuya/TuyaService.h`;
- `Services/Tuya/TuyaService.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`.

### Додано

- AES-GCM encrypt/decrypt через BearSSL;
- HMAC-SHA256 helper;
- session-key derivation для Tuya LAN `3.5`;
- `Packet6699`;
- `PREFIX = 0x00006699`;
- `SUFFIX = 0x00009966`;
- AAD = header без prefix;
- `length = IV + ciphertext + tag`;
- `SESSION_KEY_START`;
- `SESSION_KEY_RESP`;
- `SESSION_KEY_FINISH`;
- `CONTROL_NEW`;
- on-demand session negotiation перед relay-командою.

### Очікуваний runtime flow

```text
Connecting to Tuya ...
Tuya connected
Tuya: 3.5 session start sent
Tuya: 3.5 packet received, cmd=4
Tuya: 3.5 session established
Tuya: 3.5 relay command sent, dps=1 state=0
```

### Обмеження

Це перший інтеграційний етап `3.5`, тому потрібен hardware smoke-test із реальною розеткою `TCOGCZ16-A`.

### Версія

```text
0.4.13-tuya35-gcm-frame
```

---

## [0.4.12-tuya-on-demand-command] - 01.08.2026

### Статус

Виправлено модель доступності Tuya power-controller.

Раніше `TuyaPowerController::available()` означав, що TCP socket прямо зараз connected. Для Tuya LAN це нестабільно: пристрій може закривати socket після heartbeat/status query, але все одно бути доступним для нової on-demand команди.

### Оновлено

- `Services/Tuya/TuyaService.cpp`;
- `Services/Power/TuyaPowerController.cpp`;
- `Services/Power/PowerService.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`.

### Змінено

- `TuyaService::relaySet()` тепер сам виконує `connect()` перед командою, якщо socket не підключений;
- `TuyaPowerController::available()` перевіряє WiFi та валідність Tuya config, а не поточний TCP socket;
- `PowerService::restart()` більше не переходить в `Error`, якщо controller тимчасово unavailable;
- автоматичний heartbeat/status query після Tuya connect вимкнено.

### Навіщо

На Tuya LAN `3.5` status query у форматі `3.3` провокує швидкий disconnect:

```text
Tuya connected
Tuya: status query sent
Tuya disconnected
```

Тепер сервіс не тримає socket як постійний стан готовності, а відкриває з'єднання безпосередньо перед relay-командою.

### Версія

```text
0.4.12-tuya-on-demand-command
```

---

## [0.4.11-power-reconnect-wait] - 01.08.2026

### Статус

Виправлено runtime-поведінку `PowerService` після `powerOff` через Tuya LAN.

Після relay-команди Tuya TCP socket може тимчасово розірватися. Раніше `PowerService` рівно через `powerOffTime` намагався виконати `powerOn`, бачив `TuyaLan unavailable` і переходив у `Error`.

### Оновлено

- `Models/PowerData.h`;
- `Services/Power/PowerService.h`;
- `Services/Power/PowerService.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`.

### Додано

- очікування доступності power-controller перед `powerOn`;
- retry `powerOn` кожні `2000 ms`;
- timeout очікування `powerOn controller` — `60000 ms`;
- лог:

```text
PowerService: waiting for controller before power ON
```

### Результат

Тепер sequence має бути таким:

```text
PowerService: power OFF
Tuya disconnected
Tuya connected
PowerService: power ON
PowerService: restart completed
```

### Версія

```text
0.4.11-power-reconnect-wait
```

---

## [0.4.10-tuya35-detected] - 01.08.2026

### Статус

Зафіксовано, що зовнішня розетка `TCOGCZ16-A` використовує Tuya LAN protocol `3.5`.

Поточний `TuyaProtocol` підтримує тільки `3.3`, тому розетка може приймати TCP-з'єднання, але не реагувати на relay-команди.

### Оновлено

- `Services/Tuya/TuyaProtocol.h`;
- `Services/Tuya/TuyaService.cpp`;
- `Services/Config/ConfigDefaults.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`.

### Додано

- явне розпізнавання `3.4` та `3.5` як відомих, але ще не підтриманих версій;
- лог:

```text
Tuya: protocol 3.5 is detected but not supported yet
```

- конфігурація тепер приймає `tuya.version = 35`, щоб пристрій не відхиляв валідну конфігурацію.

### Висновок

Проблема з відсутністю реакції розетки не в `relayDps` як першопричині, а в несумісності поточної реалізації `3.3` з реальною Tuya LAN версією `3.5`.

Наступний етап — окрема реалізація Tuya LAN `3.5`.

### Версія

```text
0.4.10-tuya35-detected
```

---

## [0.4.9-tuya-diagnostics] - 01.08.2026

### Статус

Додано діагностику Tuya LAN команд, оскільки зовнішня розетка `TCOGCZ16-A` підключається, але не реагує на relay-команди.

### Оновлено

- `Services/Tuya/TuyaService.h`;
- `Services/Tuya/TuyaService.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`.

### Додано

- `TuyaService::queryStatus()`;
- автоматичний heartbeat після TCP connect;
- автоматичний status query після TCP connect;
- логування relay-команд:
  - `seq`;
  - `dps`;
  - `state`;
  - `bytes`;
- логування отриманих пакетів:
  - `cmd`;
  - `seq`;
  - `payload size`;
- логування помилки decrypt payload без виводу `localKey`.

### Навіщо

Цей етап дозволяє відрізнити три різні проблеми:

- розетка не відповідає взагалі;
- відповідь приходить, але payload не decrypt'иться через неправильний `localKey` або protocol version;
- відповідь приходить, але керується інший DPS, не `relayDps`.

### Версія

```text
0.4.9-tuya-diagnostics
```

---

## [0.4.8-power-controller] - 30.07.2026

### Статус

Реалізовано production-шар керування живленням через абстракцію `PowerService`.

Цей етап замінює пряму залежність `Application -> RelayService` на правильну архітектуру:

```text
WatchdogService
    -> PowerService
    -> IPowerController
    -> TuyaPowerController
    -> TuyaService
```

### Додано

- `Models/PowerData.h`;
- `Services/Power/IPowerController.h`;
- `Services/Power/PowerService.h`;
- `Services/Power/PowerService.cpp`;
- `Services/Power/TuyaPowerController.h`;
- `Services/Power/TuyaPowerController.cpp`.

### PowerService

Реалізовано:

- `setController(...)`;
- `restart(powerOffTime)`;
- неблокуючий power-cycle;
- `restartInProgress()`;
- `restartCompleted()`;
- `clearRestartCompleted()`;
- throttling повторних restart-спроб при недоступному power-controller;
- статистику restart/error;
- облік `lastPowerOn`, `lastPowerOff`, `lastRestart`, `lastError`;
- стан `PowerState`.

### TuyaPowerController

Реалізовано adapter над `TuyaService`:

```cpp
powerOn()  -> TuyaLan.relayOn()
powerOff() -> TuyaLan.relayOff()
```

Глобальний екземпляр:

```cpp
TuyaPowerController TuyaPower;
```

### Application

Оновлено runtime flow:

```cpp
TuyaLan.begin();
Power.setController(TuyaPower);
Power.begin();
```

У `loop()`:

```cpp
TuyaLan.loop();
Watchdog.update(HealthCheck.info());
Watchdog.loop();

if (Watchdog.restartRequired() && !Power.restartInProgress())
{
    Power.restart(Watchdog.data().configuration.powerOffTime);
}

Power.loop();

if (Power.restartCompleted())
{
    Power.clearRestartCompleted();
    Watchdog.restartCompleted();
}
```

### RelayService

`RelayService` залишається як legacy/promіжний GPIO-модуль, але більше не є production power-controller для `TCOGCZ16-A`.

### Обмеження

- `PowerService` залежить від доступності Tuya LAN device;
- якщо `TuyaLan` не підключений, restart не буде виконано;
- потрібен hardware smoke-test із реальним `TCOGCZ16-A`;
- Tuya LAN `3.4` ще не підтримується.

### Версія

Поточна інтеграційна версія:

```text
0.4.8-power-controller
```

Production target залишається:

```text
1.0.0
```

### Наступний етап

Наступний етап:

```text
Hardware smoke-test
```

Потрібно перевірити:

- Tuya LAN connection;
- `relayDps`;
- `powerOff`;
- `powerOn`;
- повний `Watchdog -> PowerService -> TuyaPowerController -> TuyaLan` цикл.

---

## [0.4.7-tuya-service] - 30.07.2026

### Статус

Інтегровано `TuyaProtocol` у `TuyaService`.

На цьому етапі `TuyaService` вже вміє підключатися до Tuya LAN пристрою, будувати relay command через protocol layer, відправляти binary packet у TCP socket і приймати відповіді через stream buffer.

### Оновлено

- `Services/Tuya/TuyaService.h`;
- `Services/Tuya/TuyaService.cpp`.

### TuyaService

Додано:

- `Tuya::Protocol m_protocol`;
- receive buffer розміром `Tuya::MAX_PACKET_SIZE`;
- sequence counter;
- protocol initialization з `Config.data().tuya`;
- TCP stream parser;
- packet boundary detection;
- packet size validation;
- `processPacket(...)`;
- encrypted payload decrypt через `TuyaProtocol`;
- JSON payload parse;
- оновлення `TuyaStatus.relayState` з DPS.
- глобальний екземпляр сервісу зафіксовано як `TuyaLan`, щоб не конфліктувати з namespace `Tuya`.

### Команди

`relaySet(...)` тепер виконує реальну команду:

```cpp
TuyaProtocol::buildSetDps(...)
WiFiClient::write(...)
```

та оновлює:

```cpp
commandCount
relayState
errorCount
```

### Connection lifecycle

- `begin()` стартує reconnect timer.
- `connect()` перевіряє Tuya config і protocol readiness.
- `disconnect()` очищає receive buffer і запускає reconnect delay.
- `loop()` обробляє reconnect, TCP state та receive stream.

### Обмеження

- `TuyaService` ще не підключений до `Application`;
- `WatchdogService` ще не використовує `TuyaService`;
- heartbeat/status query ще не викликаються періодично;
- response parser поки обробляє тільки JSON payload із `dps`;
- Tuya LAN `3.4` ще не підтримується.

### Версія

Поточна інтеграційна версія:

```text
0.4.7-tuya-service
```

Production target залишається:

```text
1.0.0
```

### Наступний етап

Наступний модуль:

```text
PowerController abstraction
```

Потрібно замінити GPIO-based `RelayService` на абстракцію:

```text
WatchdogService
    -> PowerService
    -> IPowerController
    -> TuyaPowerController
    -> TuyaService
```

---

## [0.4.6-tuya-protocol] - 30.07.2026

### Статус

Реалізовано перший рівень `TuyaProtocol` поверх `TuyaCrypto` та `TuyaPacket`.

Це ще не повна інтеграція з `TuyaService`, але вже готовий protocol builder/parser для основних команд Tuya LAN `3.3`.

### Додано

- `Services/Tuya/TuyaProtocol.h`;
- `Services/Tuya/TuyaProtocol.cpp`.

### TuyaProtocol

Реалізовано:

- ініціалізацію через:

```cpp
begin(deviceId, localKey, protocolVersion)
```

- перевірку готовності `ready()`;
- `buildHeartbeat(...)`;
- `buildStatusQuery(...)`;
- `buildSetDps(...)`;
- `decryptPayload(...)`;
- формування Tuya `3.3` version header;
- AES-encrypted JSON payload через `TuyaCrypto`;
- пакування результату в `Tuya::Packet`.

### Підтримувана версія

На цьому етапі явно підтримується:

```text
Tuya LAN protocol 3.3
```

`3.4` поки не реалізовано, тому що для нього потрібен окремий session-key handshake/HMAC flow. Його не додаємо як заглушку, щоб не створити фальшиве відчуття сумісності.

### Команди

Підготовлено побудову:

- heartbeat;
- status query;
- DPS relay command:

```json
{
  "devId": "...",
  "uid": "...",
  "t": "...",
  "dps": {
    "1": true
  }
}
```

Номер DPS береться з конфігурації як `relayDps`.

### Обмеження

- `TuyaService` ще не використовує `TuyaProtocol`;
- receive buffer / stream parser ще не інтегровано;
- статус relay ще не оновлюється з відповіді пристрою;
- `WatchdogService` ще не підключений до `TuyaService`.

### Версія

Поточна інтеграційна версія:

```text
0.4.6-tuya-protocol
```

Production target залишається:

```text
1.0.0
```

### Наступний етап

Наступний модуль:

```text
TuyaService integration
```

Потрібно реалізувати:

- `TuyaService::sendCommand()` через `TuyaProtocol`;
- TCP receive buffer;
- parse packet;
- decrypt response payload;
- оновлення `TuyaStatus`;
- інтеграцію `WatchdogService -> TuyaService`.

---

## [0.4.5-tuya-packet] - 28.07.2026

### Статус

Реалізовано базовий Tuya LAN packet framing.

Цей етап закриває низькорівневу роботу з binary packet envelope: header, payload, CRC32 та suffix. Payload encryption/decryption залишається окремим шаром і буде підключено на наступному етапі `TuyaProtocol`.

### Оновлено

- `Services/Tuya/TuyaPacket.cpp`.

### TuyaPacket

Реалізовано:

- `Packet::Packet()`;
- `Packet::clear()`;
- `Packet::parse(...)`;
- `Packet::build(...)`;
- big-endian читання/запис `uint32_t`;
- перевірку `PREFIX = 0x000055AA`;
- перевірку `SUFFIX = 0x0000AA55`;
- перевірку packet length;
- перевірку максимального payload size;
- CRC32 IEEE для packet integrity.

### Формат пакета

Підтримується структура:

```text
prefix   : 4 bytes
sequence : 4 bytes
command  : 4 bytes
length   : 4 bytes
payload  : N bytes
crc32    : 4 bytes
suffix   : 4 bytes
```

`length` формується як:

```text
payloadLength + 8
```

тобто включає `crc32 + suffix`.

### Обмеження

- Це ще не повний Tuya LAN protocol.
- Payload encryption/decryption ще не підключено до packet layer.
- Tuya 3.4 session/HMAC особливості ще не реалізовані.

### Версія

Поточна інтеграційна версія:

```text
0.4.5-tuya-packet
```

Production target залишається:

```text
1.0.0
```

### Наступний етап

Наступний модуль:

```text
TuyaProtocol
```

Потрібно реалізувати:

- encrypted JSON payload build;
- encrypted JSON payload parse;
- DPS command для `relayDps`;
- heartbeat packet;
- status query packet;
- зв'язку `TuyaService -> TuyaProtocol`.

---

## [0.4.4-tuya-crypto] - 28.07.2026

### Статус

Розпочато перехід від локального GPIO-реле до керування зовнішнім Tuya LAN реле `TCOGCZ16-A`.

Попередній `RelayService` через `digitalWrite()` визнано непридатним для реального hardware-сценарію. Для цього пристрою потрібен мережевий power-controller через Tuya LAN protocol.

### Додано

- `Services/Tuya/TuyaCrypto.h`;
- `Services/Tuya/TuyaCrypto.cpp`.

### TuyaCrypto

Реалізовано базовий криптографічний шар для Tuya LAN protocol `3.3/3.4`:

- AES-128-ECB;
- PKCS#7 padding;
- decrypt із перевіркою padding;
- MD5 helper;
- hex encoder;
- constant-time byte comparison;
- робота з 16-byte `localKey`;
- без додаткових PlatformIO dependencies.

### Реалізація

Для AES використовується вбудований BearSSL із ESP8266 Arduino Core.

Оскільки BearSSL надає CBC API, режим ECB реалізовано через обробку кожного 16-byte блоку окремо з нульовим IV. Це еквівалентно AES-ECB для окремих блоків і не створює CBC chaining між блоками.

### Config

Синхронізовано поточну Tuya-конфігурацію:

```json
"tuya": {
  "ip": "",
  "port": 6668,
  "deviceId": "",
  "localKey": "",
  "version": 33,
  "relayDps": 1
}
```

### Перевірено

Виконано локальну compile-перевірку на `d1_mini`:

```text
pio run
SUCCESS
```

Пам'ять після збірки тестового проєкту:

```text
RAM:   40.1%
Flash: 32.6%
```

### Версія

Поточна інтеграційна версія:

```text
0.4.4-tuya-crypto
```

Production target залишається:

```text
1.0.0
```

### Наступний етап

Наступний модуль:

```text
TuyaPacket + TuyaProtocol
```

Потрібно реалізувати:

- Tuya packet framing;
- payload encryption/decryption;
- control command для DPS relay;
- heartbeat;
- status query;
- інтеграцію `WatchdogService -> TuyaService`.

---

## [0.4.3-relay-integration] - 27.07.2026

### Статус

Етап інтеграції `Relay/PowerService` завершено.

Це перший реліз, у якому `WatchdogService` з'єднано з фізичним виконавчим модулем живлення. `WatchdogService` залишається decision-layer, а `RelayService` відповідає тільки за керування GPIO-реле та виконання power-cycle.

### Додано

- `Models/RelayData.h`;
- `Services/Relay/RelayService.h`;
- `Services/Relay/RelayService.cpp`;
- глобальний сервіс:

```cpp
RelayService Relay;
```

### RelayService

- Реалізовано керування живленням:
  - `powerOn()`;
  - `powerOff()`;
  - `restart(powerOffTime)`.
- Реалізовано неблокуючий power-cycle через `Timer`.
- Реалізовано прапор завершення циклу:

```cpp
restartCompleted()
clearRestartCompleted()
```

- Додано захист від повторного запуску restart-циклу, якщо попередній ще виконується.
- Додано статистику:
  - кількість restart-циклів;
  - час останнього `powerOn`;
  - час останнього `powerOff`.

### Application

- `Application` тепер запускає `Relay.begin()`.
- У головний цикл додано зв'язку:

```cpp
Watchdog.restartRequired()
    -> Relay.restart(powerOffTime)
    -> Relay.loop()
    -> Relay.restartCompleted()
    -> Watchdog.restartCompleted()
```

- `WatchdogService` не залежить напряму від `RelayService`.
- `RelayService` не залежить напряму від `WatchdogService`.

### Config

- Додано секцію конфігурації:

```json
"relay": {
  "enabled": false,
  "pin": 5,
  "activeHigh": true
}
```

- Додано `RelayConfig` у `Config.h`.
- Синхронізовано `ConfigDefaults.cpp`.
- Синхронізовано `ConfigJson.cpp`.
- Додано валідацію GPIO pin.
- `CONFIG_VERSION` залишено `3` для backward compatibility, щоб не примушувати пристрій скидати існуючий `/config.json` із WiFi-налаштуваннями.

### Безпека

- За замовчуванням `relay.enabled = false`.
- Це зроблено навмисно, щоб перше оновлення прошивки не могло випадково перемкнути фізичне реле.
- Для hardware-тесту реле потрібно явно увімкнути в `/config.json`.

### Версія

Поточна інтеграційна версія:

```text
0.4.3-relay-integration
```

Production target залишається:

```text
1.0.0
```

### Наступний етап

Наступний модуль:

```text
Hardware validation + relay polarity test
```

Потрібно підтвердити:

- правильний GPIO pin;
- правильну полярність `activeHigh`;
- що `powerOffTime` відповідає реальному обладнанню;
- що Watchdog не входить у restart-loop.

---

## [0.4.2-watchdog-integration] - 27.07.2026

### Статус

Етап інтеграції `WatchdogService` завершено на рівні decision-layer.

Це не фізичний power-cycle реліз. На цьому етапі `WatchdogService` аналізує `HealthCheckInfo`, визначає необхідність перезапуску та виставляє `restartPending`. Фізичне керування живленням буде винесено в наступний модуль `Relay/PowerService`.

### Додано

- `Models/WatchdogData.h`;
- `Services/Watchdog/WatchdogService.h`;
- `Services/Watchdog/WatchdogService.cpp`;
- глобальний сервіс:

```cpp
WatchdogService Watchdog;
```

### Application

- `Application` тепер запускає `Watchdog.begin()`.
- У головний цикл додано:

```cpp
Watchdog.update(HealthCheck.info());
Watchdog.loop();
```

- Спрощено `Application.h` і `Application.cpp`.
- Прибрано приватні helper-методи `initializeServices()` та `processServices()`, щоб уникнути linker-помилок при неповному копіюванні `.cpp`.

### WatchdogService

- Реалізовано state machine:
  - `Idle`;
  - `Monitoring`;
  - `RestartRequired`;
  - `Cooldown`;
  - `LockedOut`.
- Реалізовано аналіз стану `HealthCheck`.
- Реалізовано визначення `restartRequired()`.
- Реалізовано `restartCompleted()` для майбутньої інтеграції з `Relay/PowerService`.
- Реалізовано cooldown через `bootDelay`.
- Додано захист від перевищення `maxRestartPerDay`.
- Додано `LockedOut` стан при досягненні денного ліміту перезапусків.

### Config

- Синхронізовано `ConfigDefaults.cpp` з поточним `Config.h`.
- Додано дефолтне значення `pingTimeout`.
- Додано валідацію `pingTimeout`.
- Синхронізовано `ConfigJson.cpp`.
- `pingTimeout` тепер читається з `/config.json`.
- `pingTimeout` тепер зберігається у `/config.json`.

### Виправлено

- Linker-помилку:

```text
undefined reference to Application::processServices()
```

- Несумісність старого `WatchdogService` з поточним `WatchdogConfig`.
- Використання неіснуючих полів:
  - `watchdog.enabled`;
  - `watchdog.restartDelay`;
  - `watchdog.cooldownTime`.

### Версія

Поточна інтеграційна версія:

```text
0.4.2-watchdog-integration
```

Production target залишається:

```text
1.0.0
```

### Наступний етап

Наступний модуль:

```text
Relay/PowerService
```

Він повинен виконати фізичний цикл:

```text
Watchdog.restartRequired()
    -> powerOff()
    -> wait powerOffTime
    -> powerOn()
    -> Watchdog.restartCompleted()
```

---

## [0.4.1-integration-stable] - 19.07.2026

### Статус

Етап `v0.4.1 Integration Stable` завершено.

Це стабільна інтеграційна контрольна точка після реального запуску на ESP8266. Проєкт ще не є фінальним `v1.0.0 Production`, але базовий runtime вже підтверджено на пристрої.

### Підтверджено на hardware

- `Logger`: працює.
- `Storage / LittleFS`: монтується.
- `Config`: завантажується з `/config.json`.
- `Application`: проходить повний старт.
- `WiFiService`: підключається до WiFi.
- `HealthCheckService`: стартує.
- `IcmpHealthCheckProvider`: підключений через Dependency Injection.
- `IcmpSession`: виконує ICMP перевірку.
- `HealthCheck`: переходить у `ONLINE`.

Контрольний лог:

```text
Storage: LittleFS mounted
Configuration loaded
Configuration ready
WiFi: connected
HealthCheck: ONLINE
Application: started
```

### Виправлено

- Додано `Storage.begin()` перед `Config.begin()`.
- Виправлено неможливість створення `/config.json` при першому запуску.
- Виправлено помилку `Health was not declared in this scope`.
- Додано `HealthCheck.setProvider(IcmpProvider)`.
- Узгоджено `Log.begin(74880)` з `monitor_speed = 74880`.
- Прибрано сирий SDK-вивід:

```text
ping 1, timeout 0, total payload ...
```

- Для приглушення SDK ping output додано no-op `sentCallback()` у `IcmpSession`.
- Прибрано залежність `ESP8266Ping` з `platformio.ini`.

### Оновлено

- `Application.cpp`;
- `IcmpSession.h`;
- `IcmpSession.cpp`;
- `IcmpHealthCheckProvider.h`;
- `IcmpHealthCheckProvider.cpp`;
- `Storage.cpp`;
- `SystemInfo.cpp`;
- `WiFiService.h`;
- `WiFiService.cpp`;
- `platformio.ini`;
- `.gitignore`;
- `config.json`;
- `Version.h`;
- `BuildInfo.h`.

### Версія

Поточна інтеграційна версія:

```text
0.4.1-integration-stable
```

Production target залишається:

```text
1.0.0
```

### Наступний етап

Наступний великий модуль:

```text
WatchdogService + Relay/PowerService
```

---

## [0.4.0-integration] - 18.07.2026

### Статус

Інтеграційний реліз після великого архітектурного рефакторингу.

Основна мета версії - стабілізувати базову архітектуру, синхронізувати сервіси між собою та підготувати проєкт до завершення `WatchdogService`.

### Core

- Оновлено `Application.cpp`.
- Виправлено помилку компіляції `Health was not declared in this scope`.
- Замість неіснуючого глобального сервісу `Health` використовується `HealthCheck`.
- Перед запуском `HealthCheck` додано явне підключення провайдера `IcmpProvider`.
- Узгоджено порядок запуску сервісів:
  - `Logger`;
  - `Storage`;
  - `Config`;
  - `Network`;
  - `System`;
  - `HealthCheck`.

### Logger

- Logger синхронізовано з новою архітектурою сервісів.
- Підтримуються рівні `Error`, `Warning`, `Info`, `Debug`, `Verbose`.
- Додано підтримку `printf`-стилю.
- Додано підтримку RAM-рядків `const char*`.
- Додано підтримку Flash-рядків `F(...)`.

### Storage

- `Storage` синхронізовано з новим `Logger`.
- Прибрано логування через конкатенацію `String`.
- Повідомлення переведено на форматований вивід.
- Покращено діагностику помилок LittleFS та JSON.

### Models

- Додано каталог `Models`.
- Додано `Common.h`, `SystemData.h`, `NetworkData.h`, `HealthCheckData.h`, `ConfigData.h`.
- Прийнято правило: не зберігати Arduino `String` у моделях.
- `SystemData` переведено на фіксовані `char[]`.
- `NetworkData` відв'язано від `IPAddress`.
- Додано власний тип `IPv4Address`.

### SystemInfo

- `SystemInfoService` синхронізовано з `SystemData`.
- Сервіс більше не відповідає за форматований вивід у Serial.
- Прибрано метод `print()`.
- Прибрано залежність від `Logger`.

### WiFiService

- `WiFiService` синхронізовано з `NetworkData`.
- Глобальний екземпляр сервісу зафіксовано як `Network`.
- Прибрано конфлікт із глобальним Arduino/ESP8266 об'єктом `WiFi`.
- IP-адреси зберігаються у власній моделі `IPv4Address`.
- Прибрано логування через `String`.

### Network

- Додано базовий мережевий рівень `Network`.
- Додано `NetworkTypes.h`, `NetworkResult.h`, `INetworkSession.h`.
- Додано `IcmpSession`.
- `IcmpSession` реалізовано через native ESP8266 SDK `ping_start()`.
- ICMP працює асинхронно через lwIP callback.
- Прибрано стару blocking-логіку `executePing`.

### HealthCheck

- Оновлено `HealthCheckService`.
- Додано інтеграцію через `IHealthCheckProvider`.
- Додано `IcmpHealthCheckProvider`.
- `IcmpHealthCheckProvider` підключає `IcmpSession` до `HealthCheckService`.
- Зафіксовано глобальні екземпляри:

```cpp
HealthCheckService HealthCheck;
IcmpHealthCheckProvider IcmpProvider;
```

---

## Production Target

### [1.0.0] - planned

`v1.0.0` залишається цільовим production-релізом ESP Watchdog.

До нього повинні увійти:

- стабільний `Application`;
- стабільний `Logger`;
- стабільний `Config`;
- стабільний `Storage`;
- стабільний `WiFiService`;
- стабільний `SystemInfo`;
- стабільний `HealthCheck`;
- завершений `WatchdogService`;
- завершений `RelayService`;
- hardware smoke-test на ESP8266.
