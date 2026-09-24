# ESP Watchdog — Project Status

Дата: 21.09.2026

## Поточний статус

Проєкт випущено як `v1.0.0 Production` після успішної 7-денної long-run validation. Базовий runtime, HealthCheck, Watchdog decision-layer, Tuya LAN power-control, Web Dashboard / Web API шар, Tuya LAN `3.5` DPQuery, WiFi AP recovery та hardware verification baseline реалізовані й підтверджені поточним release baseline.

Основний напрямок роботи зараз:

- long-run monitoring production-пристрою;
- збір `resetInfo` лише у разі нового `Exception` reset;
- контроль heap / fragmentation / RuntimeGuard;
- перевірка деградації Web Dashboard після тривалого uptime;
- перевірка WiFi reconnect та WiFi AP recovery після втрати домашньої мережі;
- перевірка стабільності Tuya LAN `3.5` command path після idle-period;
- bugfix/stability only;
- maintenance only до наступного feature-релізу `1.1.0`.

Поточний production baseline: `v1.0.0 Production`.

Активна версія для перевірки: `v1.1.0-rc.1-observability`.

### RC scope — Persistent Restart History

- додано LittleFS persistence для bounded restart-history ring buffer;
- історія переживає ESP reboot;
- незавершений power-cycle після reset позначається як `interrupted`;
- жоден запис не виконується з hot path `loop()` без події restart.
- core persistence перевірено на пристрої: запис manual restart зберігся після software reboot ESP.
- Dashboard та authenticated API дозволяють очистити history окремо від конфігурації.
- clear action hardware-verified: history snapshot після команди порожній.

### RC scope — Availability History

- додано RAM-only bounded history для HealthCheck (`8` записів);
- зберігаються лише failure-події та зміни доступності Online / Offline;
- подія містить uptime, NTP epoch, статус, RTT і лічильник consecutive failures; epoch для подій до NTP sync обчислюється ретроспективно;
- детальні записи доступні через `GET /api/health` та Dashboard;
- authenticated `POST /api/health/history/clear` і Dashboard action очищують лише RAM history, не торкаючись HealthCheck statistics;
- `/api/status` навмисно містить тільки `capacity` і `count` history — без `entries`, щоб не ризикувати переповненням буфера відповіді на ESP8266;
- flash persistence не використовується: нова діагностика не збільшує зношення LittleFS.
- hardware verification: `network_unavailable` і наступний `success` / Online коректно потрапили в history; до NTP sync подія має `epoch=0`, після sync — валідний epoch.
- hardware verification NTP backfill: ранній запис з `epoch=0` ретроспективно отримав epoch після синхронізації часу.
- hardware verification clear action: Dashboard виконав `availability_history_clear` з `HTTP 200`.

### Наступний етап

Focused RC long-run validation для `1.1.0-rc.1-observability`: контролювати heap/fragmentation, Web UI, HealthCheck, обидва history-журнали та NTP epoch backfill протягом щонайменше 24 годин.

Початковий RC snapshot підтверджено на пристрої: startup log backfill має валідний wall time, `HealthCheck` записав initial Online event з epoch, а очищений persistent restart history стартує порожнім.

Основні документи поточного етапу:

- `LongRunTest.md`;
- `LongRunReport.md`.

### Документація

Canonical release та status документи зберігаються лише в корені репозиторію. Папка `docs/` містить технічні матеріали (`Architecture.md`, `CodingStyle.md`) та короткий `docs/README.md` index без копій Changelog, Roadmap, ProjectStatus або ReleaseChecklist.

## Вже зроблено

### Core

- створено `Application`;
- створено `IService`;
- створено неблокуючий `Timer`;
- додано `Version.h`;
- додано `BuildInfo.h`;
- `Application` переведено на глобальні сервіси:
  - `Log`;
  - `Config`;
  - `Network`;
  - `System`;
  - `HealthCheck`;
- виправлено помилку з неіснуючим глобальним сервісом `Health`;
- активний HealthCheck provider підключено через `HealthCheck.setProvider(TcpProvider)`;
- узгоджено швидкість `Logger` із `monitor_speed = 74880`.

### Logger

- реалізовано `Logger` з рівнями:
  - `Error`;
  - `Warning`;
  - `Info`;
  - `Debug`;
  - `Verbose`;
- додано підтримку форматування у стилі `printf`;
- додано підтримку RAM-рядків `const char*`;
- додано підтримку Flash-рядків `F(...)`;
- усунуто конфлікти з викликами `Log.info(...)`, `Log.warning(...)`, `Log.error(...)`.

### Storage

- `Storage` синхронізовано з новим `Logger`;
- прибрано логування через конкатенацію `String`;
- повідомлення переведено на формат `printf`;
- покращено діагностику помилок файлової системи та JSON.

### Models

- створено каталог `Models`;
- створено `Common.h`;
- створено `SystemData.h`;
- створено `NetworkData.h`;
- створено `HealthCheckData.h`;
- створено `ConfigData.h`;
- прийнято правило: у моделях не зберігати `String`;
- `SystemData` переведено на фіксовані `char[]`;
- `NetworkData` відв'язано від Arduino `IPAddress`;
- додано власний тип `IPv4Address`.

### SystemInfo

- `SystemInfoService` синхронізовано з `SystemData`;
- сервіс більше не відповідає за форматований вивід;
- прибрано `print()` із сервісу;
- прибрано прямі залежності від `Logger`;
- дані прошивки, пам'яті, CPU та uptime зберігаються в `SystemData`.

### WiFiService

- `WiFiService` синхронізовано з новою моделлю `NetworkData`;
- глобальний екземпляр сервісу зафіксовано як `Network`;
- прибрано конфлікт із глобальним `ESP8266WiFi::WiFi`;
- додано AP recovery policy для unattended режиму:
  - якщо `wifi.ssid` порожній, setup portal залишається активним для первинного налаштування;
  - якщо `wifi.ssid` заданий, але домашня мережа недоступна, AP працює 5 хвилин як rescue portal;
  - після 5 хвилин пристрій повторно пробує підключитися до домашньої WiFi-мережі;
  - якщо підключення не відновилось протягом 20 хвилин, ESP виконує контрольований restart;
- прибрано логування через `String`;
- IP-адреси переводяться у власний `IPv4Address`;
- додано збереження:
  - стану підключення;
  - hostname;
  - SSID;
  - IP;
  - gateway;
  - subnet;
  - DNS;
  - RSSI;
  - якості сигналу;
  - статистики перепідключень.

### Network

- створено базову архітектуру `Network/Common`;
- створено `NetworkTypes.h`;
- створено `NetworkResult.h`;
- створено `INetworkSession.h`;
- створено фінальну архітектуру `IcmpSession`;
- ICMP реалізація переведена на native ESP8266 SDK `ping_start()`;
- прибрано залежність від старого blocking `executePing`;
- ICMP працює як асинхронна сесія через lwIP callback;
- додано `TcpHealthCheckProvider`;
- активний HealthCheck provider у `Application` змінено на `TcpProvider`;
- production HealthCheck тепер виконується через TCP connect до `watchdog.targetHost:watchdog.targetPort`;
- для SSH-based перевірки використовується `watchdog.targetPort = 22`.

### HealthCheck

- створено `HealthCheckService`;
- створено `HealthCheckResult`;
- створено `HealthCheckInfo`;
- створено `IHealthCheckProvider`;
- створено `IcmpHealthCheckProvider`;
- створено `TcpHealthCheckProvider`;
- `HealthCheckService` працює через Dependency Injection;
- `IcmpHealthCheckProvider` підключає `IcmpSession` до сервісу HealthCheck як fallback;
- `TcpHealthCheckProvider` використовується як активний production provider;
- виправлено проблему з неоголошеним `Health`;
- прийнято глобальний екземпляр:
  - `HealthCheckService HealthCheck`;
  - `IcmpHealthCheckProvider IcmpProvider`;
  - `TcpHealthCheckProvider TcpProvider`.

### WatchdogService

- створено `Models/WatchdogData.h`;
- створено `WatchdogService.h`;
- створено `WatchdogService.cpp`;
- реалізовано decision-layer;
- реалізовано `restartRequired()`;
- реалізовано `restartCompleted()`;
- додано захист від перевищення `maxRestartPerDay`;
- додано cooldown через `bootDelay`.

### Relay / PowerService

- GPIO-based `RelayService` створено як проміжний етап, але після уточнення hardware визнано непридатним для `TCOGCZ16-A`;
- керування живленням потрібно виконувати через Tuya LAN protocol;
- `RelayService` не вважається фінальним power-controller для цього проєкту.

### PowerService

- створено `Models/PowerData.h`;
- створено `IPowerController`;
- створено `PowerService`;
- створено `TuyaPowerController`;
- `Application` переведено з `RelayService` на `PowerService`;
- `PowerService` виконує неблокуючий restart-cycle;
- `PowerService` веде in-memory restart history;
- `PowerService` логує завершені restart history entries;
- `PowerService` надає API-ready `PowerStatusData` snapshot;
- `HealthCheckService` надає API-ready `HealthStatusData` snapshot;
- `WatchdogService` надає API-ready `WatchdogStatusData` snapshot;
- `SystemInfoService` надає API-ready `SystemStatusData` snapshot;
- `WiFiService` надає API-ready `NetworkStatusData` snapshot;
- `Application` надає агрегований API-ready `ApiStatusData` snapshot;
- додано JSON serializer для `ApiStatusData`;
- додано `WebServerService` з endpoint `GET /api/status`;
- додано lightweight diagnostics endpoint `GET /api/diagnostics`;
- додано `RuntimeGuardService` для self-recovery при degraded heap / fragmentation;
- `/api/diagnostics` показує `runtimeGuard` state;
- додано `OtaService` для оновлення прошивки по WiFi;
- додано PlatformIO environment `d1_mini_ota`;
- додано lightweight Web Dashboard на `/`;
- додано subsystem API endpoints;
- додано API index endpoint `/api`;
- dashboard отримав links до API endpoints;
- додано API power commands:
  - `POST /api/power/on`;
  - `POST /api/power/off`;
  - `POST /api/power/restart`;
- dashboard отримав power-control buttons:
  - `ON`;
  - `OFF`;
  - `RESTART`;
- dashboard отримав command log;
- dashboard отримав restart history log;
- dashboard/config/logs отримали mobile-friendly layout;
- Web UI buttons і navigation links адаптовано для touch UI;
- footer API endpoints винесено в collapsible блок;
- додано route fallback для Web UI сторінок `/config/...`, `/dashboard/...`, `/logs/...`;
- Web API security baseline:
  - state-changing commands підтримують `Authorization: Bearer <token>`;
  - `WebServerService` явно збирає `Authorization` header;
  - query-token fallback прибрано;
  - misconfigured auth блокує небезпечні команди;
  - setup portal не блокується auth-перевіркою;
- додано read-only config endpoint `GET /api/config`;
- dashboard показує налаштування контрольованого хоста;
- dashboard показує налаштування Tuya socket з masked `localKey`;
- додано `POST /api/config` для оновлення всіх секцій `config.json`;
- dashboard отримав `Configuration editor`;
- `ConfigService` отримав `updateFromJson(...)` з validation-before-save;
- секрети `wifi.password` і `tuya.localKey` не віддаються відкрито через Web API;
- налаштування винесено на окремі Web-сторінки:
  - `/config/device`;
  - `/config/wifi`;
  - `/config/watchdog`;
  - `/config/relay`;
  - `/config/tuya`;
- додано `POST /api/system/restart`;
- dashboard отримав кнопку `Restart ESP`;
- `Logger` отримав runtime ring-buffer на останні 32 записи;
- додано `GET /api/logs`;
- додано окрему сторінку `/logs`;
- додано first-boot WiFi setup portal;
- якщо `wifi.ssid` порожній, ESP піднімає AP `ESP-Watchdog-Setup`;
- якщо підключення до WiFi завершується timeout, ESP переходить у setup portal;
- setup portal більше не залишається активним назавжди після втрати домашньої WiFi-мережі;
- setup portal доступний за адресою `192.168.4.1`;
- у setup mode `Application.loop()` не запускає HealthCheck/Watchdog/Power-cycle логіку;
- `PowerService` має throttling повторних restart-спроб при недоступному Tuya LAN controller;
- `PowerService` чекає перепідключення Tuya LAN controller перед `powerOn`, замість негайного переходу в `Error`;
- `TuyaPowerController` керує живленням через `TuyaLan.relayOn()` / `TuyaLan.relayOff()`.
- `TuyaPowerController::available()` більше не залежить від поточного TCP socket стану;
- `TuyaService::relaySet()` підключається до Tuya device on-demand перед відправкою relay-команди;
- автоматичний status query після connect вимкнено, щоб не провокувати disconnect на Tuya LAN `3.5`;
- додано явну політику Tuya status polling:
	  - `tuya.statusPollingEnabled`;
	  - `tuya.statusPollingInterval`;
	  - polling disabled by default;
	  - для `3.5` polling використовує encrypted `6699 DPQueryNew`.
- додано Web API authentication для state-changing endpoints:
  - `security.apiAuthEnabled`;
  - `security.apiToken`;
  - `Authorization: Bearer <token>`;
  - no query-token fallback;
  - `/config/security`;
  - browser-local token storage.

### Tuya

- у проєкті з'явився каталог `Services/Tuya`;
- створено заготовки:
  - `TuyaService.h`;
  - `TuyaService.cpp`;
  - `TuyaPacket.h`;
  - `TuyaPacket.cpp`;
- реалізовано:
  - `TuyaCrypto.h`;
  - `TuyaCrypto.cpp`;
- реалізовано:
  - `TuyaPacket.cpp`;
- реалізовано:
  - `TuyaProtocol.h`;
  - `TuyaProtocol.cpp`;
- оновлено:
  - `TuyaService.h`;
  - `TuyaService.cpp`;
- `TuyaCrypto` компілюється на ESP8266 `d1_mini`;
- реалізовано AES-128-ECB + PKCS#7 як базу для Tuya LAN protocol `3.3/3.4`.
- `TuyaPacket` реалізує binary framing, CRC32, prefix/suffix validation та payload extraction.
- `TuyaProtocol` будує heartbeat, status query та DPS control payload для Tuya LAN `3.3`.
- `TuyaService` використовує `TuyaProtocol` для `relaySet()` і має TCP receive buffer.
- глобальний екземпляр Tuya-сервісу: `TuyaLan`.
- після TCP connect `TuyaService` не відправляє автоматичний status query;
- додано діагностичні логи `seq`, `cmd`, `payload`, `dps`, `bytes` для перевірки реакції Tuya LAN device.
- підтверджено, що цільова розетка використовує Tuya LAN `3.5`;
- додано базовий Tuya LAN `3.5` frame layer `6699`;
- додано AES-GCM encrypt/decrypt;
- додано session-key negotiation START/RESP/FINISH;
- додано `CONTROL_NEW` relay command path для `3.5`;
- додано safe opt-in status polling policy;
- додано Tuya LAN `3.5` status DPQuery через encrypted `6699 DPQueryNew`;
- `3.5` status polling потребує hardware verification на цільовій розетці.

## Поточні готові файли для інтеграції

У каталозі `outputs/` підготовлено актуальні версії:

- `Application.cpp`;
- `Version.h`;
- `BuildInfo.h`;
- `platformio.ini`;
- `Changelog.md`;
- `Config.h`;
- `ConfigDefaults.cpp`;
- `ConfigJson.cpp`;
- `config.json`;
- `IcmpSession.h`;
- `IcmpSession.cpp`;
- `IcmpHealthCheckProvider.h`;
- `IcmpHealthCheckProvider.cpp`;
- `Storage.cpp`;
- `SystemInfo.cpp`;
- `WiFiService.h`;
- `WiFiService.cpp`;
- `WatchdogData.h`;
- `WatchdogService.h`;
- `WatchdogService.cpp`;
- `RelayData.h`;
- `RelayService.h`;
- `RelayService.cpp`;
- `PowerData.h`;
- `IPowerController.h`;
- `PowerService.h`;
- `PowerService.cpp`;
- `TuyaPowerController.h`;
- `TuyaPowerController.cpp`;
- `TuyaCrypto.h`;
- `TuyaCrypto.cpp`;
- `TuyaPacket.h`;
- `TuyaPacket.cpp`;
- `TuyaProtocol.h`;
- `TuyaProtocol.cpp`;
- `TuyaService.h`;
- `TuyaService.cpp`.
- `JsonStatusSerializer.h`;
- `JsonStatusSerializer.cpp`;
- `ApiStatusData.h`;
- `SystemStatusData.h`;
- `NetworkStatusData.h`;
- `HealthStatusData.h`;
- `WatchdogStatusData.h`;
- `PowerStatusData.h`;
- `RestartHistoryData.h`;
- `WebServerService.h`;
- `WebServerService.cpp`;

## Відомі ризики

- повна збірка проєкту після копіювання всіх файлів ще потребує перевірки;
- `POST /api/config` змінює файл конфігурації, але частина сервісів може потребувати reboot для повного застосування нових налаштувань;
- Web configuration editor поки не має authentication layer;
- power-control buttons на dashboard виконують реальні команди Tuya socket;
- частина старих файлів може містити застарілі include-шляхи;
- GPIO-based `RelayService` не відповідає реальному hardware `TCOGCZ16-A`;
- `PowerService` перевірено на реальному Tuya LAN пристрої;
- якщо `TuyaLan` недоступний, restart не буде виконано;
- Tuya protocol `3.4` поки не підтримується;
- Tuya protocol `3.5` hardware-verified на `TCOGCZ16-A`;
- потрібні реальні `ip`, `deviceId`, `localKey`, `version`, `relayDps`;
- Tuya LAN `3.5` status DPQuery через `6699` потребує hardware verification;
- `localKey` не можна логувати або дублювати у відкритих звітах;
- `security.apiToken` не можна логувати або дублювати у відкритих звітах;
- потрібно перевірити, що Serial Monitor і `Log.begin(...)` використовують однакову швидкість;
- потрібно поступово винести форматування в `Formatters/LogFormatter`.

## Наступні кроки

1. Hardware-verify TCP/SSH HealthCheck на реальному контрольованому хості.
2. Hardware-verify Tuya LAN `3.5` status DPQuery через `6699`.
3. Перевірити OTA update на RC build.
4. Підтримувати 7d+ production monitoring та збирати diagnostics після кожної аномалії.
5. Планувати лише non-breaking improvements для `1.1.0`.
