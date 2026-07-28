# ESP Watchdog

**ESP Watchdog** — автономний апаратний watchdog для віддаленого перезапуску серверів та іншого мережевого обладнання.

Проєкт побудований на **ESP8266 (WeMos D1 mini)** і працює незалежно від Home Assistant або будь-якого іншого сервера.

---

## Основні можливості

* Автоматичний контроль доступності сервера.
* Перевірка доступності за допомогою ICMP Ping.
* Автоматичний перезапуск живлення через Wi-Fi розетку.
* Незалежна робота без Home Assistant.
* Веб-інтерфейс для налаштування.
* OTA оновлення прошивки.
* Автоматичне перепідключення до Wi-Fi.
* Зберігання конфігурації у LittleFS.
* Детальна система логування.
* Модульна архітектура.

---

# Апаратна платформа

* ESP8266 WeMos D1 mini
* Wi-Fi 2.4 GHz
* LittleFS
* PlatformIO
* Arduino Framework

---

# Призначення

Основна задача пристрою — автоматично відновлювати роботу обладнання у випадках, коли воно перестає відповідати по мережі.

Типовий сценарій:

```
Home Assistant
        │
        │ Ping
        ▼
 ESP Watchdog
        │
        │ HTTP / Tuya
        ▼
 Wi-Fi Smart Plug
        │
        ▼
 Home Assistant Power
```

У випадку втрати зв'язку:

1. Виконується декілька перевірок Ping.
2. Якщо сервер недоступний — вимикається Wi-Fi розетка.
3. Через заданий час живлення знову подається.
4. Watchdog очікує завершення завантаження сервера.
5. Контроль продовжується.

---

# Архітектура

```
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
└── OTA
```

Кожен сервіс має однаковий життєвий цикл:

```
begin()

loop()

data()
```

---

# Структура проєкту

```
src/

Application.*

Logger.*

Storage.*

Config.*

SystemInfo.*

WiFiService.*

Timer.*

PingService.*

WatchdogService.*

WebServer.*

OTAService.*

Version.h
```

---

# Принципи розробки

* Один сервіс — одна відповідальність.
* Мінімальна зв'язаність між модулями.
* Відсутність блокуючих `delay()`.
* Використання машин станів (State Machine).
* Використання `constexpr` замість `#define`.
* Єдиний стиль іменування.
* Повторне використання компонентів.
* Простота тестування окремих модулів.

---

# Використані бібліотеки

* ESP8266 Arduino Core
* ArduinoJson
* LittleFS

У майбутньому:

* ESP8266Ping
* ESPAsyncWebServer (або альтернативний HTTP Server)
* ArduinoOTA

---

# План розвитку

## v0.1

* Logger
* Storage
* Config

## v0.2

* SystemInfo

## v0.3

* WiFiService

## v0.4

* Timer

## v0.5

* PingService

## v0.6

* PlugController

## v0.7

* WatchdogService

## v0.8

* WebServer

## v0.9

* OTA

## v1.0

Перша стабільна версія.

---

# Ліцензія

Проєкт розробляється як відкритий модульний watchdog для ESP8266.

---

# Автор

Vasyl Dotsenko
