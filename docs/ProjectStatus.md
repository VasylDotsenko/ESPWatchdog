# ESP Watchdog — Project Status

Дата: 30.07.2026

## Поточний статус

Проєкт знаходиться у стані інтеграційної стабілізації після завершення базового runtime, HealthCheck, Watchdog decision-layer та інтеграції PowerController abstraction для Tuya LAN керування зовнішнім реле `TCOGCZ16-A`.

Основний напрямок роботи зараз:

- стабілізація Tuya LAN `3.5` runtime;
- фіналізація Tuya status polling policy;
- очищення runtime-логів;
- підготовка до наступних production-модулів: Web API, diagnostics, restart history export.

Проєкт ще не є фінальним production-релізом. Поточний стан позначено як `v0.4.25-api-status-snapshot`.

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
- додано підключення `IcmpProvider` через `HealthCheck.setProvider(IcmpProvider)`;
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
- ICMP працює як асинхронна сесія через lwIP callback.

### HealthCheck

- створено `HealthCheckService`;
- створено `HealthCheckResult`;
- створено `HealthCheckInfo`;
- створено `IHealthCheckProvider`;
- створено `IcmpHealthCheckProvider`;
- `HealthCheckService` працює через Dependency Injection;
- `IcmpHealthCheckProvider` підключає `IcmpSession` до сервісу HealthCheck;
- виправлено проблему з неоголошеним `Health`;
- прийнято глобальний екземпляр:
  - `HealthCheckService HealthCheck`;
  - `IcmpHealthCheckProvider IcmpProvider`.

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
- `PowerService` має throttling повторних restart-спроб при недоступному Tuya LAN controller;
- `PowerService` чекає перепідключення Tuya LAN controller перед `powerOn`, замість негайного переходу в `Error`;
- `TuyaPowerController` керує живленням через `TuyaLan.relayOn()` / `TuyaLan.relayOff()`.
- `TuyaPowerController::available()` більше не залежить від поточного TCP socket стану;
- `TuyaService::relaySet()` підключається до Tuya device on-demand перед відправкою relay-команди;
- автоматичний status query після connect вимкнено, щоб не провокувати disconnect на Tuya LAN `3.5`.

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
- після TCP connect `TuyaService` відправляє heartbeat та status query;
- додано діагностичні логи `seq`, `cmd`, `payload`, `dps`, `bytes` для перевірки реакції Tuya LAN device.
- підтверджено, що цільова розетка використовує Tuya LAN `3.5`;
- додано базовий Tuya LAN `3.5` frame layer `6699`;
- додано AES-GCM encrypt/decrypt;
- додано session-key negotiation START/RESP/FINISH;
- додано `CONTROL_NEW` relay command path для `3.5`.

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

## Відомі ризики

- повна збірка проєкту після копіювання всіх файлів ще потребує перевірки;
- частина старих файлів може містити застарілі include-шляхи;
- GPIO-based `RelayService` не відповідає реальному hardware `TCOGCZ16-A`;
- `PowerService` перевірено на реальному Tuya LAN пристрої;
- якщо `TuyaLan` недоступний, restart не буде виконано;
- Tuya protocol `3.4` поки не підтримується;
- Tuya protocol `3.5` hardware-verified на `TCOGCZ16-A`;
- потрібні реальні `ip`, `deviceId`, `localKey`, `version`, `relayDps`;
- `localKey` не можна логувати або дублювати у відкритих звітах;
- потрібно перевірити, що Serial Monitor і `Log.begin(...)` використовують однакову швидкість;
- потрібно поступово винести форматування в `Formatters/LogFormatter`.

## Наступні кроки

1. Зафіксувати `v0.4.25-api-status-snapshot` як hardware-verified baseline.
2. Реалізувати Tuya status polling policy.
3. Додати restart history Web/API export.
4. Додати Web API для status/config/health/watchdog/power.
5. Додати diagnostics endpoint.
