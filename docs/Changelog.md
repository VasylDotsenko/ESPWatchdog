# ESP Watchdog

> Журнал змін проєкту

---

## [0.5.3-ota-update] - 20.08.2026

### Статус

Додано OTA update для оновлення WeMos D1 mini по WiFi.

### Додано

- `OtaService.h`;
- `OtaService.cpp`;
- інтеграцію `OTA.begin()` та `OTA.loop()` в `Application`;
- PlatformIO environment:

```ini
[env:d1_mini_ota]
extends = env:d1_mini
upload_protocol = espota
upload_port = 192.168.10.44
```

### Поведінка

- OTA service стартує тільки після підключення до домашньої WiFi-мережі;
- у setup portal mode OTA не активується;
- hostname береться з `Config.data().device.hostname`;
- якщо `security.apiAuthEnabled=true` і `security.apiToken` заданий, той самий токен використовується як OTA password;
- під час OTA upload `RuntimeGuard` не виконує recovery restart;
- OTA progress логується кожні 10%.

### Важливо

Першу прошивку з OTA підтримкою потрібно один раз залити через USB. Після цього наступні оновлення можна виконувати по мережі.

### Версія

```text
0.5.3-ota-update
```

---

## [0.5.2-runtime-recovery-guard] - 20.08.2026

### Статус

Додано runtime recovery guard для довготривалої роботи ESP8266.

### Причина

Після приблизно двох діб uptime WeMos D1 mini може поступово сповільнюватися і втрачати функціональність. Power-cycle пристрою відновлює роботу. Типові причини для ESP8266:

- деградація heap;
- heap fragmentation;
- завислі TCP/Web/Tuya ресурси;
- довготривалі побічні ефекти WiFi/LwIP;
- важкі runtime JSON/Web запити на малому heap.

### Додано

- `RuntimeGuardService`;
- інтеграцію `RuntimeGuard` в `Application`;
- runtime guard state у `/api/diagnostics`;
- контрольований restart тільки самого ESP, без power-cycle зовнішньої розетки.

### Recovery policy

Guard не реагує на одиничні провали. Restart планується тільки якщо:

- uptime більше 1 години;
- пристрій не в setup portal mode;
- PowerService не виконує restart-cycle;
- `freeHeap < 8000` bytes або `heapFragmentation > 60%`;
- деградація триває не менше 10 хвилин.

Після цього виконується delayed `ESP.restart()` через 3000 ms.

### Diagnostics

`GET /api/diagnostics` тепер додатково повертає:

- `runtimeGuard.enabled`;
- `runtimeGuard.degraded`;
- `runtimeGuard.restartScheduled`;
- `runtimeGuard.freeHeap`;
- `runtimeGuard.heapFragmentation`;
- `runtimeGuard.minFreeHeap`;
- `runtimeGuard.maxHeapFragmentation`;
- `runtimeGuard.degradedSince`;
- `runtimeGuard.restartAt`.

### Версія

```text
0.5.2-runtime-recovery-guard
```

---

## [0.5.1-diagnostics-baseline] - 10.08.2026

### Статус

Додано lightweight diagnostics endpoint для польового debug.

### Додано

- `WebApiDiagnostics.h`;
- `WebApiDiagnostics.cpp`;
- endpoint:

```text
GET /api/diagnostics
```

- link `/api/diagnostics` у API index;
- link `diagnostics` у footer Web UI.

### Diagnostics snapshot

Endpoint повертає компактний JSON без restart history:

- `ok`;
- `level`: `ok`, `warn`, `bad`;
- system:
  - `freeHeap`;
  - `heapFragmentation`;
  - `uptimeSeconds`;
  - `resetReason`;
  - heap/fragmentation warning flags;
- network:
  - connected;
  - RSSI;
  - quality;
  - reconnect count;
- health:
  - available;
  - running;
  - response time;
  - sent/lost;
  - consecutive failures;
- watchdog:
  - enabled;
  - restart pending;
  - locked out;
  - cooldown;
  - restart count;
- power:
  - available;
  - restart in progress;
  - restart count;
  - error count.

### Чому

`/api/status` уже містить повний aggregate snapshot, але для польової діагностики потрібен короткий endpoint, який швидко відповідає на питання: чи вистачає heap, чи є WiFi, чи живий HealthCheck, чи Watchdog готує restart, і чи PowerService/Tuya має помилки.

### Версія

```text
0.5.1-diagnostics-baseline
```

---

## [0.5.0-security-baseline] - 09.08.2026

### Статус

Перший production hardening етап для Web API security.

### Оновлено

- state-changing Web API endpoints залишаються відкритими тільки якщо `security.apiAuthEnabled=false`;
- якщо `security.apiAuthEnabled=true`, команди приймають токен тільки через:

```text
Authorization: Bearer <token>
```

- видалено fallback авторизації через query parameter `?token=...`;
- `WebServerService` явно збирає `Authorization` header через `collectHeaders(...)`;
- якщо auth увімкнено, але `security.apiToken` порожній, state-changing commands блокуються з:

```json
{"ok":false,"error":"api_auth_misconfigured"}
```

- у setup portal mode авторизація не блокує первинне налаштування;
- read-only endpoints залишаються відкритими для Dashboard/status/diagnostics.

### Чому

Token у URL може потрапляти в browser history, logs, reverse proxy logs або screenshots. Для production firmware безпечніше використовувати стандартний `Authorization` header і явно блокувати небезпечні команди, якщо auth увімкнено некоректно.

### Версія

```text
0.5.0-security-baseline
```

---

## [0.4.61-web-config-route-fallback] - 09.08.2026

### Статус

Hotfix для відкриття Web UI config routes.

### Оновлено

- додано fallback у `WebServerService::handleNotFound()` для:
  - `/config`;
  - `/config/...`;
  - `/dashboard/...`;
  - `/logs/...`;
- `/config/tuya` та майбутні config routes тепер повинні віддавати SPA-сторінку навіть якщо URL має trailing slash або route не був явно зареєстрований;
- API routes зберігають JSON `404`.

### Чому

Після переходу на окремі Web UI сторінки config route має поводитись як SPA entrypoint. Це прибирає крихкість exact-match routes на ESP8266WebServer і робить сторінки налаштувань стабільнішими.

### Версія

```text
0.4.61-web-config-route-fallback
```

---

## [0.4.60-web-ui-mobile-polish] - 09.08.2026

### Статус

Завершено responsive/mobile polish для Web UI.

### Оновлено

- dashboard, config pages і logs адаптовано для вузьких екранів;
- збільшено touch targets для кнопок і navigation links;
- кнопки дій на mobile тепер займають повну ширину;
- `input` поля отримали `font-size: 16px`, щоб уникнути автоматичного zoom на iOS;
- покращено spacing cards/rows на малих екранах;
- довгі значення у рядках статусу краще переносяться;
- `API endpoints` у footer винесено в collapsible `details/summary`;
- log panel отримав більшу висоту на mobile;
- Web UI залишився lightweight, без додаткових залежностей.

### Чому

Після появи окремих сторінок конфігурації та runtime logs інтерфейс став використовуватись не лише з desktop, а й з телефону під час налаштування WeMos у локальній мережі. Цей реліз робить базові дії зручнішими на touch UI без збільшення складності фронтенду.

### Версія

```text
0.4.60-web-ui-mobile-polish
```

---

## [0.4.59-config-apply-policy] - 09.08.2026

### Статус

Додано базову policy застосування конфігурації.

### Оновлено

- `POST /api/config` тепер повертає:
  - `section`;
  - `applyMode`;
  - `requiresRestart`;
  - `restartRecommended`;
- додано section-level policy:
  - `security` застосовується live;
  - `device`, `wifi`, `watchdog`, `relay`, `tuya` потребують restart ESP;
- Web UI показує `restart required` для секцій, які не застосовуються live;
- Web UI показує `live applied` для security config;
- command log тепер відображає apply result замість загального повідомлення.

### Чому

Після появи Web UI config editor потрібно чітко розрізняти зміни, які застосовуються одразу, і зміни, які потребують reboot. Це зменшує неоднозначність у польовій експлуатації й готує основу для майбутнього live apply окремих секцій.

### Версія

```text
0.4.59-config-apply-policy
```

---

## [0.4.58-web-ui-production-polish] - 09.08.2026

### Статус

Перший етап production polish для Web UI.

### Оновлено

- command log більше не показує сирий JSON для power/config/restart команд;
- додано компактне форматування результатів Web API command responses;
- після `POST /api/config` UI показує `restart recommended`;
- configuration pages показують помітний warning-блок, якщо після save рекомендовано restart ESP;
- після save config Dashboard оновлює cached config;
- `Restart ESP` у UI показує scheduled delay і переходить у стан `reconnecting...`;
- додано захист від невалідного JSON у command responses;
- UI залишено lightweight без великих зовнішніх залежностей.

### Чому

Після стабілізації Web API основна проблема була не в функціональності, а в польовому UX: користувач бачив сирі JSON-відповіді й не мав зрозумілого сигналу, що після зміни конфігурації потрібен restart. Цей етап робить Web UI ближчим до production-панелі керування.

### Версія

```text
0.4.58-web-ui-production-polish
```

---

## [0.4.57-web-command-auth-cleanup] - 09.08.2026

### Статус

Восьмий етап розділення `WebServerService.cpp`.

### Оновлено

- додано спільний Web API response helper:
  - `WebApiResponse.h`;
  - `WebApiResponse.cpp`;
- додано окремий command authorization helper:
  - `WebApiAuth.h`;
  - `WebApiAuth.cpp`;
- CORS/cache/JSON headers централізовано в `WebApiResponse`;
- command authorization винесено з `WebServerService.cpp`;
- `tokenMatches()` перенесено в `WebApiAuth`;
- `WebApiStatus`, `WebApiIndex`, `WebApiConfig`, `WebApiPower`, `WebApiLogs` синхронізовано зі спільними response helpers;
- `WebServerService.cpp` зменшено приблизно до `491` рядка.

### Чому

Після розділення Web API модулів однакова логіка відповіді (`sendJson`, CORS headers, cache headers) почала дублюватися в кількох файлах. Централізація response/auth helpers зменшує ризик різної поведінки між endpoints і готує Web layer до security hardening у `0.5.x`.

### Версія

```text
0.4.57-web-command-auth-cleanup
```

---

## [0.4.56-webserver-route-cleanup] - 09.08.2026

### Статус

Сьомий етап розділення `WebServerService.cpp`.

### Оновлено

- route registration згруповано за відповідальністю:
  - page routes;
  - read-only status API routes;
  - config/logs API routes;
  - command API routes;
  - utility routes;
- додано окремі файли:
  - `WebApiIndex.h`;
  - `WebApiIndex.cpp`;
- handler `GET /api` винесено з `WebServerService.cpp`;
- `WebServerService::configureRoutes()` тепер є коротким coordinator-методом;
- `WebServerService.cpp` став читабельнішим і ближчим до ролі route coordinator-а.

### Чому

Після винесення сторінок, config API, logs API, power commands і status API найбільшим джерелом шуму залишався великий блок реєстрації routes. Групування routes робить файл простішим для ревʼю і зменшує ризик помилок під час додавання нових endpoints.

### Версія

```text
0.4.56-webserver-route-cleanup
```

---

## [0.4.55-web-api-status-split] - 09.08.2026

### Статус

Шостий етап розділення `WebServerService.cpp`.

### Оновлено

- read-only status API винесено з `WebServerService.cpp`;
- додано окремі файли:
  - `WebApiStatus.h`;
  - `WebApiStatus.cpp`;
- `WebApiStatus` відповідає за:
  - `GET /api/status`;
  - `GET /api/system`;
  - `GET /api/network`;
  - `GET /api/health`;
  - `GET /api/watchdog`;
  - `GET /api/power`;
- `WebServerService` залишає за собою route registration, CORS/options, authorization для command endpoints і page serving.

### Чому

Read-only status endpoints є окремою відповідальністю presentation/API layer. Після винесення `WebServerService.cpp` стає ближчим до ролі coordinator-а, а status serialization можна підтримувати окремо від config, logs і power commands.

### Версія

```text
0.4.55-web-api-status-split
```

---

## [0.4.54-dashboard-fast-load] - 09.08.2026

### Статус

Оптимізація швидкості відкриття Dashboard на ESP8266.

### Оновлено

- Dashboard спочатку пробує завантажити один агрегований endpoint `/api/status`;
- split endpoints залишені як fallback, якщо `/api/status` тимчасово недоступний;
- `/api/config` більше не блокує перший рендер головної сторінки;
- конфігурація Dashboard кешується на 30 секунд;
- auto-refresh зменшено з `2000 ms` до `5000 ms`;
- додано guard від одночасних накладених `load()` циклів у браузері.

### Чому

Попередній варіант був дуже стабільним, але повільним: Dashboard робив багато окремих HTTP-запитів до ESP8266. Новий режим робить перший екран значно швидшим і зменшує навантаження на WebServer.

### Версія

```text
0.4.54-dashboard-fast-load
```

---

## [0.4.53-dashboard-resilient] - 09.08.2026

### Статус

Hotfix доступності Dashboard після restart ESP та при частковій недоступності API.

### Виправлено

- додано явний route `GET /dashboard`;
- Dashboard більше не залежить критично від `/api/config`;
- read-only status endpoints на Dashboard завантажуються незалежно один від одного;
- якщо один endpoint тимчасово недоступний, головна сторінка все одно відкривається і показує доступні блоки;
- повідомлення про помилку тепер показує конкретний endpoint / HTTP status.

### Чому

Після restart або при увімкненому API auth endpoint `/api/config` міг бути недоступний для браузера без токена. Через це весь Dashboard переходив у стан `unable to load`, хоча базові status endpoints працювали.

### Версія

```text
0.4.53-dashboard-resilient
```

---

## [0.4.52-esp-restart-safe] - 09.08.2026

### Статус

Hotfix стабільності для restart ESP з Web UI.

### Виправлено

- збільшено затримку перед `ESP.restart()` після `POST /api/system/restart` з `500 ms` до `2000 ms`;
- HTTP-відповідь тепер встигає повернутися браузеру до фактичного restart;
- прибрано зайвий лог безпосередньо перед `ESP.restart()`;
- перед restart додано короткий settle/yield, щоб TCP/WebServer стек ESP8266 встиг коректно завершити поточний цикл.

### Чому

На ESP8266 швидкий restart одразу після HTTP command може падати в `Exception (29)` / `ctx: sys`, бо системний TCP/WiFi контекст ще активний. Restart тепер планується як відкладена команда, а не як агресивний reset одразу після обробки request.

### Версія

```text
0.4.52-esp-restart-safe
```

---

## [0.4.51-web-api-power-split] - 09.08.2026

### Статус

П'ятий етап розділення `WebServerService.cpp`.

### Оновлено

- manual power command handlers винесено з `WebServerService.cpp`;
- додано окремі файли:
  - `WebApiPower.h`;
  - `WebApiPower.cpp`;
- `WebServerService` залишає за собою authorization;
- `WebApiPower` відповідає за:
  - `POST /api/power/on`;
  - `POST /api/power/off`;
  - `POST /api/power/restart`;
  - parsing `powerOffTime`;
  - JSON responses для power commands.

### Чому

Power command logic є окремою відповідальністю. Після винесення `WebServerService` стає ближчим до ролі route coordinator-а, а manual power control легше тестувати й підтримувати.

### Версія

```text
0.4.51-web-api-power-split
```

---

## [0.4.50-web-api-logs-split] - 09.08.2026

### Статус

Четвертий етап розділення `WebServerService.cpp`.

### Оновлено

- handler `GET /api/logs` винесено з `WebServerService.cpp`;
- додано окремі файли:
  - `WebApiLogs.h`;
  - `WebApiLogs.cpp`;
- streaming JSON для runtime logs збережено без зміни поведінки;
- сторінка `/logs` продовжує використовувати той самий endpoint;
- `WebServerService` більше не залежить напряму від `WebJsonUtils`.

### Чому

`/api/logs` уже був оптимізований для ESP8266 через chunked response. Винесення його в окремий модуль зменшує розмір `WebServerService.cpp` і залишає сервісу роль routing/coordinator.

### Версія

```text
0.4.50-web-api-logs-split
```

---

## [0.4.49-web-api-config-split] - 09.08.2026

### Статус

Третій етап розділення `WebServerService.cpp`.

### Оновлено

- handler-и `/api/config` винесено з `WebServerService.cpp`;
- додано окремі файли:
  - `WebApiConfig.h`;
  - `WebApiConfig.cpp`;
- `WebServerService` залишає за собою routing та authorization;
- `WebApiConfig` відповідає за:
  - streaming `GET /api/config`;
  - `POST /api/config`;
  - маскування секретів для Web UI;
  - формування JSON-відповідей конфігурації;
- поведінка Web UI/API не змінювалась.

### Чому

`/api/config` є одним із найбільших Web API handler-ів. Винесення його в окремий модуль зменшує розмір `WebServerService.cpp` і робить подальше обслуговування сторінок налаштувань безпечнішим.

### Версія

```text
0.4.49-web-api-config-split
```

---

## [0.4.48-web-json-utils] - 09.08.2026

### Статус

Другий етап розділення `WebServerService.cpp`.

### Оновлено

- JSON escaping винесено з `WebServerService.cpp`;
- додано окремі файли:
  - `WebJsonUtils.h`;
  - `WebJsonUtils.cpp`;
- streaming endpoints `/api/config` та `/api/logs` тепер використовують:

```cpp
WebJsonUtils::sendEscaped(...)
```

- поведінка Web UI та API не змінювалась.

### Чому

`WebServerService` не повинен містити низькорівневі helper-и форматування JSON. Винесення `sendEscaped()` готує код до подальшого розділення на `WebApiConfig`, `WebApiLogs` і command handlers.

### Версія

```text
0.4.48-web-json-utils
```

---

## [0.4.47-web-pages-split] - 09.08.2026

### Статус

Перший етап розділення `WebServerService.cpp` після стабілізації Web UI.

### Оновлено

- вбудований HTML/JS Dashboard винесено з `WebServerService.cpp`;
- додано окремі файли:
  - `WebPages.h`;
  - `WebPages.cpp`;
- `WebServerService` тепер відповідає за маршрути та HTTP handlers, а не зберігає великий HTML-документ;
- поведінка Web UI не змінювалась.

### Чому

`WebServerService.cpp` став занадто великим і складним для підтримки. Винесення сторінки в окремий модуль — безпечний перший крок до подальшого розділення Web API, сторінок і command handlers.

### Версія

```text
0.4.47-web-pages-split
```

---

## [0.4.46-dashboard-light] - 09.08.2026

### Статус

Оптимізація Web Dashboard після стабілізації сторінок конфігурації.

### Оновлено

- Dashboard більше не використовує великий агрегований endpoint `/api/status`;
- Web UI завантажує статус окремими меншими запитами:
  - `/api/system`;
  - `/api/network`;
  - `/api/health`;
  - `/api/watchdog`;
  - `/api/power`;
  - `/api/config`;
- запити виконуються послідовно, без одночасного навантаження на ESP8266 WebServer;
- зменшено ризик heap/stack проблем під час регулярного оновлення Dashboard.

### Чому

`/api/status` формує великий агрегований JSON. Для ESP8266 стабільніше віддавати менші JSON-відповіді та не запускати кілька важких HTTP handler-ів одночасно.

### Версія

```text
0.4.46-dashboard-light
```

---

## [0.4.45-config-pages-light] - 09.08.2026

### Статус

Hotfix для падіння при відкритті сторінок конфігурації, зокрема `/config/watchdog`.

### Виправлено

- сторінки `/config/...` більше не роблять паралельний запит до `/api/status`;
- для редакторів конфігурації Web UI тепер завантажує тільки `/api/config`;
- зменшено навантаження на ESP8266 WebServer handler-и під час відкриття сторінок налаштувань;
- знижено ризик `Exception (29)` / `ctx: sys` при відкритті `/config/watchdog`.

### Чому

Для сторінки налаштувань дані `/api/status` не потрібні. Одночасне завантаження `/api/status` і `/api/config` створювало зайве навантаження на heap/stack ESP8266 саме в момент відкриття сторінки.

### Версія

```text
0.4.45-config-pages-light
```

---

## [0.4.44-web-config-stream] - 09.08.2026

### Статус

Hotfix стабільності Web UI після виправлення сторінки логів.

### Виправлено

- `/api/config` більше не створює проміжний `JsonDocument` для всієї конфігурації;
- відповідь конфігурації тепер віддається потоково через `sendContent()`;
- рядкові поля конфігурації проходять через безпечне JSON-escaping;
- зменшено пікове використання RAM під час відкриття сторінок налаштувань.

### Чому

Сторінки налаштувань Web UI регулярно звертаються до `/api/config`. На ESP8266 краще не формувати великі JSON-документи всередині HTTP handler-а, якщо відповідь можна безпечно віддати частинами.

### Версія

```text
0.4.44-web-config-stream
```

---

## [0.4.43-web-logs-stream] - 09.08.2026

### Статус

Hotfix для crash `Stack smashing detected` при відкритті сторінки логів.

### Виправлено

- `/api/logs` більше не створює повний `JsonDocument` для runtime log buffer;
- JSON для логів тепер віддається потоково через chunked response;
- кожен `LogEntry` читається і відправляється окремо;
- додано безпечне JSON-escaping для повідомлень логера;
- зменшено пікове використання RAM/stack при відкритті `/logs`.

### Чому

Сторінка логів відкривала `/api/logs`, де одночасно формувався весь JSON зі списком логів. Для ESP8266 це ризиковано, особливо коли WebServer, Logger і WiFi працюють в одному циклі. Потокова відповідь прибирає великий проміжний JSON-об'єкт і робить сторінку логів значно стабільнішою.

### Версія

```text
0.4.43-web-logs-stream
```

---

## [0.4.42-stack-safe-logs] - 09.08.2026

### Статус

Hotfix для `Stack smashing detected`, який існував ще до переходу на TCP/SSH HealthCheck.

### Виправлено

- прибрано великий стековий масив:

```cpp
LogEntry entries[Logger::LOG_CAPACITY]
```

з `WebServerService::handleApiLogs()`;

- додано читання runtime log ring-buffer по одному запису:
  - `Logger::count()`;
  - `Logger::entry(...)`;
- `/api/logs` більше не копіює весь log buffer на стек;
- очищено випадково пошкоджений `IcmpSession::begin()`.

### Чому

На ESP8266 стек дуже обмежений. `Logger::LOG_CAPACITY = 32`, а один `LogEntry` містить message buffer. Копіювання всіх записів у локальний масив усередині HTTP handler-а могло займати кілька KB стеку й провокувати `Stack smashing detected`.

### Версія

```text
0.4.42-stack-safe-logs
```

---

## [0.4.41-web-api-auth-stack-fix] - 09.08.2026

### Статус

Hotfix для runtime crash `Stack smashing detected` після додавання Web API auth.

### Виправлено

- прибрано custom `collectHeaders("X-API-Token")`;
- прибрано локальний стековий `maskedApiToken[]` з `handleApiConfig()`;
- Web UI тепер передає токен через стандартний header:

```text
Authorization: Bearer <token>
```

- ручний fallback `?token=...` залишено.

### Чому

На ESP8266 краще використовувати стандартний `Authorization` header, який `ESP8266WebServer` підтримує штатно, замість custom header collection. Це зменшує ризик runtime crash у WebServer path.

### Версія

```text
0.4.41-web-api-auth-stack-fix
```

---

## [0.4.40-web-api-auth] - 09.08.2026

### Статус

Додано базовий захист state-changing Web API endpoints.

Read-only endpoints залишаються відкритими для dashboard і diagnostics, а небезпечні `POST` команди можуть бути захищені API token.

### Оновлено

- `Services/Config/Config.h`;
- `Services/Config/ConfigDefaults.cpp`;
- `Services/Config/ConfigJson.cpp`;
- `Services/WebServer/WebServerService.h`;
- `Services/WebServer/WebServerService.cpp`;
- `config.json`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`;
- `Roadmap.md`;
- `Changelog.md`.

### Додано

- секцію конфігурації `security`;
- `security.apiAuthEnabled`;
- `security.apiToken`;
- Web UI сторінку `/config/security`;
- browser-local token storage через кнопку `api token`;
- перевірку `X-API-Token`;
- fallback `?token=...` для curl/manual testing;
- `401 {"ok":false,"error":"unauthorized"}` для неавторизованих команд.

### Захищені endpoints

```text
POST /api/config
POST /api/system/restart
POST /api/power/on
POST /api/power/off
POST /api/power/restart
```

### Поведінка за замовчуванням

Auth вимкнений:

```json
"security": {
  "apiAuthEnabled": false,
  "apiToken": ""
}
```

Це зберігає сумісність із first-boot setup portal і старими `/config.json`.

### Версія

```text
0.4.40-web-api-auth
```

---

## [0.4.39-tcp-ssh-healthcheck] - 09.08.2026

### Статус

HealthCheck переведено з нестабільного ICMP Ping на TCP connect перевірку контрольованого хоста.

Для SSH-based перевірки використовується TCP connect до:

```text
watchdog.targetHost:watchdog.targetPort
```

Типовий production варіант:

```text
targetPort = 22
```

Це не повний SSH-клієнт і не виконує авторизацію. Перевірка вважається успішною, якщо контрольований хост приймає TCP-з'єднання на SSH-порту.

### Оновлено

- `Services/HealthCheck/TcpHealthCheckProvider.h`;
- `Services/HealthCheck/TcpHealthCheckProvider.cpp`;
- `Core/Application.cpp`;
- `Services/Config/ConfigDefaults.cpp`;
- `Services/Config/ConfigJson.cpp`;
- `config.json`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`;
- `Roadmap.md`;
- `Changelog.md`.

### Додано

- `TcpHealthCheckProvider`;
- активний provider у `Application`: `TcpProvider`;
- TCP/SSH availability check через `WiFiClient.connect()`;
- підтримка `watchdog.targetPort`;
- типовий target port змінено на `22`.

### Чому

ICMP Ping на ESP8266 / WeMos D1 mini може бути нестабільним через особливості WiFi/lwIP/SDK і поведінку роутера. TCP connect до SSH-порту краще відповідає реальному критерію доступності контрольованого хоста.

### Версія

```text
0.4.39-tcp-ssh-healthcheck
```

---

## [0.4.38-tuya-status-polling-policy] - 09.08.2026

### Статус

Додано контрольовану політику Tuya status polling.

Поточний hardware-verified сценарій для Tuya LAN `3.5` залишено стабільним: relay commands працюють on-demand, а автоматичний status query для `3.5` не виконується, доки не буде реалізовано окремий `6699 DPQuery`.

### Оновлено

- `Services/Tuya/TuyaService.h`;
- `Services/Tuya/TuyaService.cpp`;
- `Services/Config/Config.h`;
- `Services/Config/ConfigDefaults.cpp`;
- `Services/Config/ConfigJson.cpp`;
- `Services/WebServer/WebServerService.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `config.json`;
- `README.md`;
- `ProjectStatus.md`;
- `Roadmap.md`;
- `Changelog.md`.

### Додано

- `tuya.statusPollingEnabled`;
- `tuya.statusPollingInterval`;
- Web UI поля на сторінці `/config/tuya`;
- відображення polling status у configuration dashboard;
- мінімальний безпечний polling interval: `30000 ms`;
- grace period після TCP connect: `30000 ms`;
- grace period після relay command: `10000 ms`.

### Поведінка

- polling за замовчуванням вимкнений;
- для Tuya LAN `3.3` polling може бути увімкнений через `/config/tuya`;
- для Tuya LAN `3.5` автоматичний polling пропускається, щоб не провокувати disconnect;
- relay `powerOff/powerOn` path не змінено.

### Версія

```text
0.4.38-tuya-status-polling-policy
```

---

## [0.4.37-first-boot-wifi-setup] - 09.08.2026

### Статус

Додано first-boot WiFi setup portal для первинного підключення WeMos D1 mini до домашньої мережі.

Якщо `wifi.ssid` порожній або підключення до WiFi завершується timeout, пристрій піднімає власну AP-точку `ESP-Watchdog-Setup`. Через неї можна відкрити Web UI, налаштувати WiFi та інші параметри, зберегти `config.json` і перезавантажити ESP.

### Оновлено

- `Models/NetworkData.h`;
- `Services/WiFi/WiFiService.h`;
- `Services/WiFi/WiFiService.cpp`;
- `Serializers/JsonStatusSerializer.cpp`;
- `Core/Application.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`;
- `Roadmap.md`;
- `Changelog.md`.

### Додано

- `NetworkState::SetupPortal`;
- `WiFiService::setupMode()`;
- fallback AP mode:
  - SSID: `ESP-Watchdog-Setup`;
  - password: `12345678`;
  - IP: `192.168.4.1`;
- автоматичний setup portal при порожньому `wifi.ssid`;
- автоматичний setup portal після WiFi connection timeout;
- захист від Watchdog/Power-cycle у setup mode.

### Поведінка first boot

```text
config.json missing/default
        ↓
wifi.ssid empty
        ↓
ESP starts AP: ESP-Watchdog-Setup
        ↓
User opens http://192.168.4.1/config/wifi
        ↓
User saves WiFi settings
        ↓
User clicks Restart ESP
        ↓
ESP connects to home WiFi
```

### Версія

```text
0.4.37-first-boot-wifi-setup
```

---

## [0.4.36-web-logs-page-runtime-buffer] - 09.08.2026

### Статус

Додано runtime log ring-buffer у `Logger`, Web API endpoint для логів і окрему сторінку `/logs`.

Це переводить логування з Serial-only діагностики у Web-visible diagnostics layer. Тепер останні runtime logs можна переглядати з браузера без підключення Serial Monitor.

### Оновлено

- `Services/Logger/LogLevel.h`;
- `Services/Logger/Logger.h`;
- `Services/Logger/Logger.cpp`;
- `Services/WebServer/WebServerService.h`;
- `Services/WebServer/WebServerService.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`;
- `Roadmap.md`;
- `Changelog.md`.

### Додано

- ring-buffer на останні 32 log entries;
- структура `LogEntry`;
- `Logger::entries(...)`;
- `Logger::clear()`;
- `GET /api/logs`;
- окрема сторінка `GET /logs`;
- link `logs` у dashboard navigation;
- runtime logs card з timestamp, level і message.

### API

```text
GET /api/logs
GET /logs
```

### Обмеження

Log buffer зберігається тільки в RAM і очищується після reboot ESP.

### Cleanup

- `Logger::clear()` більше не використовує `memset` для `LogEntry`;
- усунуто compiler warning `-Wclass-memaccess`.

### Версія

```text
0.4.36-web-logs-page-runtime-buffer
```

---

## [0.4.35-web-config-pages-esp-restart] - 09.08.2026

### Статус

Налаштування винесено з dashboard на окремі Web-сторінки за розділами `config.json`, додано кнопку та API для перезапуску ESP.

Dashboard більше не містить одну велику config-форму. Натомість кожна секція редагується окремо, а `POST /api/config` використовує часткове оновлення JSON.

### Оновлено

- `Services/WebServer/WebServerService.h`;
- `Services/WebServer/WebServerService.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`;
- `Roadmap.md`;
- `Changelog.md`.

### Додано

- окремі сторінки налаштувань:
  - `GET /config/device`;
  - `GET /config/wifi`;
  - `GET /config/watchdog`;
  - `GET /config/relay`;
  - `GET /config/tuya`;
- section-based config editor;
- partial JSON save для кожної секції;
- `POST /api/system/restart`;
- кнопка `Restart ESP` на dashboard;
- кнопка `Restart ESP` на config-сторінках;
- неблокуючий delayed restart після HTTP-відповіді.

### API

```text
POST /api/system/restart
```

### Версія

```text
0.4.35-web-config-pages-esp-restart
```

---

## [0.4.34-web-config-editor] - 09.08.2026

### Статус

Додано Web UI для редагування всіх налаштувань з `config.json`.

Після read-only перегляду конфігурації dashboard отримав повноцінний configuration editor. Налаштування зберігаються через `POST /api/config` у LittleFS, використовуючи той самий формат, що й `/config.json`.

### Оновлено

- `Services/Config/Config.h`;
- `Services/Config/ConfigJson.cpp`;
- `Services/WebServer/WebServerService.h`;
- `Services/WebServer/WebServerService.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`;
- `Roadmap.md`;
- `Changelog.md`.

### Додано

- `ConfigService::updateFromJson(...)`;
- `POST /api/config`;
- validation-before-save для web config update;
- dashboard `Configuration editor`;
- редагування секцій:
  - `device`;
  - `wifi`;
  - `watchdog`;
  - `relay`;
  - `tuya`;
- захист від випадкового перезапису секретів masked value;
- auto-refresh pause під час редагування input-полів.

### Секрети

`wifi.password` і `tuya.localKey` не показуються відкрито.

Якщо поле пароля/ключа залишити порожнім — поточне значення зберігається.

Якщо ввести нове значення — воно буде записане в `config.json`.

### API

```text
GET  /api/config
POST /api/config
```

### Версія

```text
0.4.34-web-config-editor
```

---

## [0.4.33-web-config-display] - 09.08.2026

### Статус

Додано відображення налаштувань контрольованого хоста та Tuya-розетки у Web Dashboard.

Конфігурацію винесено в окремий read-only endpoint `GET /api/config`, щоб не змішувати runtime status і налаштування. Секретний `localKey` не віддається у відкритому вигляді — dashboard отримує тільки masked value.

### Оновлено

- `Services/WebServer/WebServerService.h`;
- `Services/WebServer/WebServerService.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`;
- `Changelog.md`.

### Додано

- `GET /api/config`;
- блок `Controlled host` на dashboard:
  - target host;
  - target port;
  - ping interval;
  - ping timeout;
  - fail count;
- блок `Tuya socket` на dashboard:
  - IP address;
  - port;
  - protocol version;
  - relay DPS;
  - device ID;
  - masked local key.

### Безпека

`tuya.localKey` не показується повністю. У Web API повертається тільки поле:

```json
"localKeyMasked": "************abcd"
```

### Версія

```text
0.4.33-web-config-display
```

---

## [0.4.32-web-power-controls-log] - 09.08.2026

### Статус

Додано ручне керування живленням безпосередньо на lightweight dashboard.

Після появи `POST` API-команд сторінка `/` отримала UI для польового тестування Tuya LAN power-cycle без `curl`: кнопки `ON`, `OFF`, `RESTART`, command log браузера і restart history з runtime status.

### Оновлено

- `Services/WebServer/WebServerService.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`;
- `Changelog.md`.

### Додано

- блок `Power controls` на dashboard;
- кнопка `ON` -> `POST /api/power/on`;
- кнопка `OFF` -> `POST /api/power/off`;
- кнопка `RESTART` -> `POST /api/power/restart`;
- browser-side `Command log`;
- `Restart history` з `power.history.entries`;
- confirm-dialog для небезпечних `OFF` і `RESTART`;
- автоматичне блокування кнопок, якщо controller unavailable або restart уже виконується.

### Версія

```text
0.4.32-web-power-controls-log
```

---

## [0.4.31-api-power-commands] - 09.08.2026

### Статус

Додано перші state-changing Web API команди для ручного керування зовнішнім Tuya LAN power controller.

Цей етап переводить Web API з read-only status layer у контрольований command layer. Команди реалізовано через `POST`, щоб не виконувати небезпечні power-операції через звичайні `GET` запити браузера.

### Оновлено

- `Models/RestartHistoryData.h`;
- `Services/Power/PowerService.h`;
- `Services/Power/PowerService.cpp`;
- `Serializers/JsonStatusSerializer.cpp`;
- `Services/WebServer/WebServerService.h`;
- `Services/WebServer/WebServerService.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`;
- `Changelog.md`.

### Додано

- `POST /api/power/on`;
- `POST /api/power/off`;
- `POST /api/power/restart`;
- `RestartReason::ManualCommand`;
- JSON-відповіді для power command API;
- CORS headers для `GET`, `POST`, `OPTIONS`;
- `OPTIONS` handlers для power command endpoints.

### Поведінка

- `POST /api/power/on` вмикає живлення через активний `IPowerController`;
- `POST /api/power/off` вимикає живлення, якщо restart-cycle не виконується;
- `POST /api/power/restart` запускає неблокуючий power-cycle;
- якщо restart вже виконується, API повертає `409`;
- якщо power controller недоступний, API повертає `503`;
- ручний restart записується в restart history як `manual_command`.

### Endpoints

```text
POST /api/power/on
POST /api/power/off
POST /api/power/restart
POST /api/power/restart?powerOffTime=10000
```

### Версія

```text
0.4.31-api-power-commands
```

---

## [0.4.30-api-index-dashboard-links] - 09.08.2026

### Статус

Додано API index endpoint і навігаційні посилання на dashboard.

Це невеликий polish-етап для зручності польового тестування: всі доступні API маршрути тепер можна побачити прямо з пристрою.

### Оновлено

- `Services/WebServer/WebServerService.h`;
- `Services/WebServer/WebServerService.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`;
- `Changelog.md`.

### Додано

- `GET /api`;
- JSON index доступних endpoints;
- links у footer dashboard:
  - `/api`;
  - `/api/status`;
  - `/api/system`;
  - `/api/network`;
  - `/api/health`;
  - `/api/watchdog`;
  - `/api/power`.

### Endpoints

```text
GET /
GET /api
GET /api/status
GET /api/system
GET /api/network
GET /api/health
GET /api/watchdog
GET /api/power
GET /health
```

### Версія

```text
0.4.30-api-index-dashboard-links
```

---

## [0.4.29-api-subsystem-endpoints] - 09.08.2026

### Статус

Додано окремі Web API endpoints для runtime subsystem-ів.

Після агрегованого `/api/status` і lightweight dashboard додано точкові endpoints, щоб діагностика могла читати окремий subsystem без завантаження повного JSON.

### Оновлено

- `Serializers/JsonStatusSerializer.h`;
- `Serializers/JsonStatusSerializer.cpp`;
- `Services/WebServer/WebServerService.h`;
- `Services/WebServer/WebServerService.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`;
- `Changelog.md`.

### Додано

- `GET /api/system`;
- `GET /api/network`;
- `GET /api/health`;
- `GET /api/watchdog`;
- `GET /api/power`;
- subsystem serializer methods:
  - `serializeSystem()`;
  - `serializeNetwork()`;
  - `serializeHealth()`;
  - `serializeWatchdog()`;
  - `serializePower()`.

### Endpoints

```text
GET /api/status
GET /api/system
GET /api/network
GET /api/health
GET /api/watchdog
GET /api/power
GET /health
```

### Версія

```text
0.4.29-api-subsystem-endpoints
```

---

## [0.4.28-web-dashboard-lite] - 09.08.2026

### Статус

Додано перший читабельний Web Dashboard.

`WebServerService` тепер віддає не лише JSON API, а й легку HTML-сторінку на `/`, яка автоматично читає `/api/status` і показує основний runtime стан у картках.

### Оновлено

- `Services/WebServer/WebServerService.h`;
- `Services/WebServer/WebServerService.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`;
- `Changelog.md`.

### Додано

- route `/`;
- embedded HTML dashboard у firmware;
- auto-refresh кожні 2 секунди;
- картки:
  - System;
  - Network;
  - Health;
  - Watchdog;
  - Power;
- посилання на `/api/status` та `/health`;
- `Cache-Control: no-store` для HTML.

### Runtime

Після WiFi connect:

```text
http://<device-ip>/
```

Для поточної плати:

```text
http://192.168.10.44/
```

JSON API лишається:

```text
http://192.168.10.44/api/status
```

### Версія

```text
0.4.28-web-dashboard-lite
```

---

## [0.4.27-webserver-api-status] - 09.08.2026

### Статус

Додано перший Web API endpoint.

`WebServerService` запускає стандартний `ESP8266WebServer` на порту `80` і віддає агрегований runtime status через `GET /api/status`.

### Оновлено

- `Services/WebServer/WebServerService.h`;
- `Services/WebServer/WebServerService.cpp`;
- `Core/Application.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`;
- `Changelog.md`.

### Додано

- `WebServerService`;
- `GET /api/status`;
- `GET /health`;
- JSON 404 response;
- CORS header `Access-Control-Allow-Origin: *`;
- `Cache-Control: no-store`;
- використання `JsonStatusSerializer` для `App.status()`.

### Runtime

Очікуваний лог:

```text
WebServer: started, port=80
```

Після WiFi connect endpoint буде доступний:

```text
http://<device-ip>/api/status
```

### Версія

```text
0.4.27-webserver-api-status
```

---

## [0.4.26-json-status-serializer] - 09.08.2026

### Статус

Додано JSON serializer для агрегованого runtime status snapshot.

Після створення `ApiStatusData` додано перший presentation-layer компонент, який перетворює clean POD snapshot у JSON для майбутнього `GET /api/status`.

### Оновлено

- `Serializers/JsonStatusSerializer.h`;
- `Serializers/JsonStatusSerializer.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`;
- `Changelog.md`.

### Додано

- `JsonStatusSerializer`;
- `JsonStatusSerializer::serialize()`;
- JSON blocks:
  - `system`;
  - `network`;
  - `health`;
  - `watchdog`;
  - `power`;
- enum text mapping для status/state/reason/result;
- IPv4 formatting;
- restart history JSON array.

### Приклад використання

```cpp
char json[4096] {};
size_t jsonLength = 0;

JsonStatusSerializer::serialize(
    App.status(),
    json,
    sizeof(json),
    jsonLength);
```

### Призначення

`JsonStatusSerializer` стане основою для:

- `GET /api/status`;
- diagnostics JSON endpoint;
- Web Dashboard;
- майбутнього MQTT telemetry payload.

### Версія

```text
0.4.26-json-status-serializer
```

---

## [0.4.25-api-status-snapshot] - 09.08.2026

### Статус

Додано агрегований API-ready status snapshot.

Після створення окремих snapshot-моделей для System, Network, Health, Watchdog і Power додано єдину модель `ApiStatusData`, яка збирає весь runtime стан проєкту в один DTO для майбутнього `GET /api/status`.

### Оновлено

- `Models/ApiStatusData.h`;
- `Core/Application.h`;
- `Core/Application.cpp`;
- `Core/Version.h`;
- `platformio.ini`;
- `README.md`;
- `ProjectStatus.md`;
- `Changelog.md`.

### Додано

- `ApiStatusData`;
- `Application::status()`.

### Містить

- `SystemStatusData`;
- `NetworkStatusData`;
- `HealthStatusData`;
- `WatchdogStatusData`;
- `PowerStatusData`.

### Призначення

`ApiStatusData` стане джерелом для:

- `/api/status`;
- dashboard overview;
- diagnostics summary;
- майбутнього MQTT telemetry summary.

### Версія

```text
0.4.25-api-status-snapshot
```

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
