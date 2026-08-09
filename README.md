# ESP Watchdog

**ESP Watchdog** — автономний мережевий watchdog на базі **ESP8266 / WeMos D1 mini** для контролю доступності обладнання та автоматичного перезапуску живлення через зовнішній Tuya LAN power controller.

Поточний інтеграційний стан:

```text
0.4.18-restart-history
```

Production target:

```text
1.0.0
```

---

## Призначення

Пристрій періодично перевіряє доступність цільового вузла через ICMP Ping. Якщо вузол не відповідає задану кількість разів поспіль, watchdog ініціює power-cycle зовнішнього реле / Wi-Fi power controller.

Цільовий hardware для power control:

```text
TCOGCZ16-A через Tuya LAN protocol
```

Типовий сценарій:

```text
Target Host
    │
    │ ICMP Ping
    ▼
ESP Watchdog
    │
    │ Tuya LAN
    ▼
TCOGCZ16-A
    │
    ▼
Power Cycle
```

---

## Поточний статус

Проєкт перебуває на етапі інтеграційної стабілізації.

Вже підтверджено:

- boot на ESP8266;
- Logger;
- LittleFS Storage;
- Config loading;
- WiFi connection;
- ICMP HealthCheck;
- HealthCheck `ONLINE`;
- Watchdog decision-layer;
- PowerService abstraction;
- PowerService waits for Tuya LAN reconnect before `powerOn`;
- Tuya relay commands connect on-demand;
- automatic Tuya status query after connect disabled for unstable 3.5 devices;
- Tuya LAN `3.5` AES-GCM `6699` frame layer;
- Tuya LAN `3.5` session-key negotiation;
- Tuya LAN `3.5` `CONTROL_NEW` relay command path;
- IPowerController;
- TuyaPowerController;
- зв'язка `WatchdogService -> PowerService -> TuyaService`;
- Tuya LAN command diagnostics;
- on-demand Tuya LAN command session;
- Tuya LAN `3.5` support for `TCOGCZ16-A`;
- hardware-verified Tuya LAN power-cycle;
- базовий Tuya LAN stack:
  - `TuyaCrypto`;
  - `TuyaPacket`;
  - `TuyaProtocol`;
  - `TuyaService`.

Ще не завершено:

- Tuya status polling policy;
- Web UI;
- OTA.

---

## Основні можливості

- неблокуюча сервісна архітектура;
- централізований `Application` lifecycle;
- конфігурація через LittleFS JSON;
- стабільний `Logger` із `printf`-style API;
- WiFi reconnect state machine;
- ICMP HealthCheck через native ESP8266 SDK ping;
- накопичення health statistics;
- Watchdog decision-layer;
- restart history у `PowerService`;
- захист від restart-loop через `maxRestartPerDay`;
- production power-control abstraction через `PowerService`;
- Tuya LAN power controller adapter;
- Tuya LAN crypto / packet / protocol / service layers;
- hardware-verified power-cycle через `TCOGCZ16-A`.

---

## Архітектура

```text
main.cpp
    │
    ▼
Application
    │
    ├── Logger
    ├── Storage
    ├── Config
    ├── WiFiService
    ├── SystemInfo
    ├── TuyaService
    ├── PowerService
    ├── HealthCheckService
    └── WatchdogService
```

Production power-control layer:

```text
WatchdogService
    │
    ▼
PowerService
    │
    ▼
IPowerController
    │
    ▼
TuyaPowerController
    │
    ▼
TuyaService
    │
    ▼
TCOGCZ16-A
```

---

## HealthCheck

```text
HealthCheckService
    │
    ▼
IHealthCheckProvider
    │
    ▼
IcmpHealthCheckProvider
    │
    ▼
IcmpSession
    │
    ▼
ESP8266 SDK ping_start()
```

HealthCheck відповідає лише за:

- запуск перевірок;
- state machine;
- статистику;
- визначення `online/offline`.

Провайдер відповідає за конкретний спосіб перевірки.

---

## Tuya LAN stack

```text
TuyaService
    │
    ▼
TuyaProtocol
    │
    ├── TuyaCrypto
    │
    └── TuyaPacket
```

### TuyaCrypto

Реалізовано:

- AES-128-ECB;
- PKCS#7 padding;
- decrypt із padding validation;
- MD5 helper;
- hex encoder;
- constant-time compare;
- робота з 16-byte `localKey`.

### TuyaPacket

Реалізовано:

- Tuya binary packet framing;
- big-endian `uint32_t`;
- `PREFIX = 0x000055AA`;
- `SUFFIX = 0x0000AA55`;
- packet length validation;
- CRC32;
- payload extraction.

### TuyaProtocol

Реалізовано для Tuya LAN `3.3`:

- heartbeat packet;
- status query packet;
- DPS control packet;
- encrypted JSON payload;
- payload decrypt.

### TuyaService

Реалізовано:

- TCP connection до Tuya device;
- reconnect timer;
- sequence counter;
- `relaySet(true/false)`;
- TCP receive buffer;
- packet parser;
- decrypt response payload;
- оновлення `TuyaStatus.relayState`.

Глобальний екземпляр:

```cpp
TuyaService TuyaLan;
```

Назва `TuyaLan` використовується навмисно, щоб не конфліктувати з namespace:

```cpp
namespace Tuya
```

---

## Конфігурація

Файл:

```text
/config.json
```

Основні секції:

```json
{
  "version": 3,
  "device": {
    "hostname": "ESP-Watchdog"
  },
  "wifi": {
    "ssid": "...",
    "password": "...",
    "reconnectInterval": 10000,
    "connectTimeout": 15000
  },
  "watchdog": {
    "targetHost": "192.168.10.50",
    "targetPort": 80,
    "pingInterval": 5000,
    "pingTimeout": 1000,
    "failCount": 5,
    "bootDelay": 120000,
    "powerOffTime": 10000,
    "maxRestartPerDay": 20
  },
  "tuya": {
    "ip": "192.168.10.xx",
    "port": 6668,
    "deviceId": "...",
    "localKey": "...",
    "version": 35,
    "relayDps": 1
  }
}
```

Важливо:

- `localKey` не логувати;
- `localKey` не публікувати;
- Tuya `3.4` поки не підтримується;
- Tuya `3.5` підтримується для `TCOGCZ16-A`;
- `localKey` має бути актуальним для конкретного пристрою.

---

## Структура проєкту

```text
src/
├── Core/
│   ├── Application.h
│   ├── Application.cpp
│   ├── IService.h
│   ├── Timer.h
│   ├── Timer.cpp
│   ├── Version.h
│   └── BuildInfo.h
│
├── Models/
│   ├── Common.h
│   ├── NetworkData.h
│   ├── SystemData.h
│   ├── HealthCheckData.h
│   ├── WatchdogData.h
│   ├── PowerData.h
│   └── RelayData.h
│
├── Network/
│   ├── Common/
│   │   ├── INetworkSession.h
│   │   ├── NetworkResult.h
│   │   └── NetworkTypes.h
│   └── Icmp/
│       ├── IcmpSession.h
│       └── IcmpSession.cpp
│
└── Services/
    ├── Config/
    ├── HealthCheck/
    ├── Logger/
    ├── Power/
    ├── Storage/
    ├── SystemInfo/
    ├── Tuya/
    ├── Watchdog/
    └── WiFi/
```

---

## Залежності

PlatformIO:

```ini
[env:d1_mini]
platform = espressif8266
board = d1_mini
framework = arduino

monitor_speed = 74880
upload_speed = 921600

board_build.filesystem = littlefs

lib_deps =
    bblanchon/ArduinoJson
```

Використовується:

- ESP8266 Arduino Core;
- ArduinoJson;
- LittleFS;
- ESP8266 native SDK ping;
- BearSSL із ESP8266 Arduino Core.

---

## Збірка

```bash
pio run
```

Заливка прошивки:

```bash
pio run --target upload
```

Заливка LittleFS:

```bash
pio run --target uploadfs
```

Serial monitor:

```bash
pio device monitor -b 74880
```

---

## Поточний roadmap

### Наступний етап

```text
Hardware smoke-test
```

Перевірено:

- Tuya LAN connection;
- правильність `relayDps`;
- `TuyaLan.relayOff()`;
- `TuyaLan.relayOn()`;
- повний цикл `WatchdogService -> PowerService -> TuyaPowerController -> TuyaService`;
- hardware smoke-test із `TCOGCZ16-A`.

### Далі

- heartbeat/status polling у `TuyaService`;
- Tuya status polling policy;
- Web API;
- Web UI;
- OTA;
- diagnostics;
- restart history Web/API export;
- average RTT;
- availability history.

---

## Поточні обмеження

- проєкт ще не є production `v1.0.0`;
- Tuya LAN `3.4` ще не підтримується;
- GPIO `RelayService` залишився як проміжний модуль, але не є фінальним рішенням для `TCOGCZ16-A`;
- TCP socket Tuya LAN може закриватися пристроєм після idle-періоду; це нормально для on-demand режиму.

---

## Автор

Vasyl Dotsenko

---

## Ліцензія

Проєкт розробляється як відкритий модульний watchdog для ESP8266.
