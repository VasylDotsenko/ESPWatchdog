# Architecture

## Загальна архітектура

ESP Watchdog побудований за модульним принципом.

Кожен компонент відповідає лише за одну задачу та не залежить від внутрішньої реалізації інших модулів.

Основною точкою входу є `Application`.

```text
                         Application
                              │
 ┌────────────────────────────┼────────────────────────────┐
 │                            │                            │
 ▼                            ▼                            ▼
Logger                     Storage                     Config
                                                         │
                                                         ▼
                                                   Configuration
                                                         │
                                                         ▼
                                                   SystemInfo
                                                         │
                                                         ▼
                                                   WiFiService
                                                         │
                                                         ▼
                                                    PingService
                                                         │
                                                         ▼
                                                  PlugController
                                                         │
                                                         ▼
                                                 WatchdogService
                                                         │
                                    ┌────────────────────┴────────────────────┐
                                    ▼                                         ▼
                              WebServer                                  OTA Update
```

---

# Життєвий цикл

Усі сервіси мають однаковий життєвий цикл.

```cpp
begin();

loop();

data();
```

## begin()

Ініціалізація сервісу.

Викликається один раз під час запуску.

---

## loop()

Основний цикл роботи.

Повинен виконуватися максимально швидко.

Не допускається використання:

* delay()
* while()
* блокуючих циклів очікування

Усі часові операції виконуються через клас `Timer`.

---

## data()

Повертає структуру із поточним станом сервісу.

Приклад:

```cpp
Config.data();

Network.data();

SystemInfo.data();

Ping.data();
```

---

# Послідовність запуску

```text
Application.begin()

│

├── Logger.begin()

├── Storage.begin()

├── Config.begin()

├── SystemInfo.begin()

├── Network.begin()

├── Ping.begin()

├── PlugController.begin()

├── Watchdog.begin()

├── WebServer.begin()

└── OTA.begin()
```

---

# Основний цикл

```text
Application.loop()

│

├── Network.loop()

├── Ping.loop()

├── PlugController.loop()

├── Watchdog.loop()

├── WebServer.loop()

└── OTA.loop()
```

---

# Залежності між сервісами

```text
Logger
    ▲
    │
Усі модулі можуть використовувати Logger.

Storage
    ▲
    │
Config

Config
    ▲
    │
Network

Network
    ▲
    │
Ping

Ping
    ▲
    │
Watchdog

Watchdog
    ▲
    │
PlugController
```

Залежності мають бути односторонніми.

Нижній рівень не повинен залежати від верхнього.

---

# Потік даних

```text
Config
   │
   ▼
Network
   │
   ▼
Ping
   │
   ▼
Watchdog
   │
   ▼
PlugController
```

---

# Структури даних

Кожен сервіс має власну структуру даних.

```cpp
ConfigData

SystemData

NetworkData

PingData

WatchdogData

StatisticsData
```

Доступ здійснюється тільки через:

```cpp
Service.data()
```

Приклад:

```cpp
Config.data().hostname;

Network.data().ip;

Ping.data().successCount;
```

---

# Машини станів

Сервіси, що працюють у фоновому режимі, використовують State Machine.

Приклад:

```cpp
enum class NetworkState
{
    Disconnected,
    Connecting,
    Connected
};
```

У майбутньому аналогічно:

```cpp
enum class PingState;

enum class WatchdogState;

enum class OTAState;

enum class WebServerState;
```

---

# Таймери

Усі часові операції виконуються тільки через клас `Timer`.

Не допускається використання:

```cpp
millis() - lastTime > interval
```

в інших сервісах.

Правильно:

```cpp
if (timer.expired())
{
    ...
}
```

---

# Логування

Усі повідомлення проходять через `Logger`.

Не допускається використання:

```cpp
Serial.print(...)
```

безпосередньо в сервісах.

Правильно:

```cpp
Log.info(...);

Log.warning(...);

Log.error(...);
```

---

# Конфігурація

Усі налаштування зберігаються у LittleFS.

Доступ здійснюється тільки через `ConfigService`.

Жоден інший модуль не працює з файлами напряму.

---

# Принципи проєкту

* Один сервіс — одна відповідальність.
* Мінімальна зв'язаність між сервісами.
* Відсутність глобальних змінних (окрім глобальних сервісів).
* Відсутність блокуючих викликів.
* Повторне використання компонентів.
* Просте тестування окремих модулів.
* Максимальна читабельність коду.
* Єдиний стиль програмування.
* Модульність і можливість розширення без зміни існуючих сервісів.

---

# Цільова архітектура v1.0

```text
Application
│
├── Logger
├── Storage
├── Config
├── SystemInfo
├── Network
├── Timer
├── Ping
├── PlugController
├── Watchdog
├── WebServer
├── OTA
└── Statistics
```

Кожен модуль є незалежним, має власний стан (`data()`), власний життєвий цикл (`begin()`, `loop()`) і взаємодіє з іншими сервісами лише через їхній публічний API.
