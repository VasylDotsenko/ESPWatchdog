# ESP Watchdog — Hardware Verification

Дата: 08.09.2026

Поточний baseline:

```text
0.9.2-long-run-test
```

Production target:

```text
1.0.0
```

---

## Мета

Підтвердити, що поточний Release Candidate стабільно працює на реальному hardware:

- ESP8266 / WeMos D1 mini;
- домашня WiFi-мережа;
- контрольований host через TCP/SSH;
- Tuya LAN розетка `TCOGCZ16-A`;
- Web Dashboard / API;
- OTA.

---

## Передумови

### Firmware

Очікувана версія:

```text
0.9.2-long-run-test
```

Перевірити у dashboard або через:

```bash
curl http://192.168.10.44/api/status
```

### Serial monitor

```bash
pio device monitor -b 74880
```

### Базові адреси

```text
ESP Watchdog:  http://192.168.10.44
Tuya socket:   192.168.10.41:6668
Target host:   watchdog.targetHost:watchdog.targetPort
```

Якщо адреси інші — використовувати фактичні значення з `/api/config`.

---

## 1. Boot Verification

### Дія

Перезапустити ESP через живлення або Web UI.

### Очікуваний Serial log

```text
Storage: LittleFS mounted
Loading configuration...
Configuration loaded
Configuration ready
WiFi: connecting to ...
TuyaService started
TuyaPowerController: started
PowerService: started, controller=TuyaPowerController
HealthCheck: started
Watchdog: started
WebServer: started, port=80
OTAService: started
Application: started
WiFi: connected, IP=...
TimeService: time synchronized
HealthCheck: ONLINE
```

### Pass

- немає `Exception`;
- немає `Stack smashing detected`;
- немає reboot loop;
- dashboard доступний після WiFi connect.

---

## 1A. WiFi AP Recovery Verification

### Мета

Перевірити, що ESP не залишається назавжди в AP mode після втрати домашньої WiFi-мережі.

### Сценарій A — перший старт без WiFi config

1. Тимчасово очистити `wifi.ssid` у `config.json`.
2. Перезапустити ESP.

Очікування:

- стартує AP `ESP-Watchdog-Setup`;
- сторінка `http://192.168.4.1/config/wifi` доступна;
- ESP не виконує автоматичний restart через 20 хвилин.

### Сценарій B — домашня WiFi-мережа недоступна

1. Залишити валідний `wifi.ssid` у `config.json`.
2. Вимкнути домашню WiFi-мережу або тимчасово заблокувати підключення ESP до роутера.
3. Перезапустити ESP.

Очікуваний потік:

```text
WiFi: connection timeout
WiFi: setup portal started, ... recovery=1
через 5 хвилин:
WiFi: setup portal timeout, retrying configured network
WiFi: connecting to ...
якщо WiFi все ще недоступний:
WiFi: recovery failed, restarting ESP, reason=wifi_recovery_timeout
```

### Pass

- AP rescue mode працює не довше 5 хвилин між спробами STA;
- якщо WiFi повернувся, ESP підключається до домашньої мережі та вимикає AP;
- якщо WiFi не повернувся, ESP виконує контрольований restart приблизно через 20 хвилин;
- Watchdog / Power-cycle не запускаються під час setup portal.

---

## 2. Diagnostics Snapshot

### Дія

Відкрити:

```text
http://192.168.10.44/api/diagnostics
```

### Очікування

```json
{
  "ok": true,
  "level": "ok",
  "system": {},
  "network": {},
  "health": {},
  "watchdog": {},
  "power": {},
  "tuya": {},
  "runtimeGuard": {}
}
```

### Pass

- JSON валідний;
- `level` не `bad`;
- `runtimeGuard.degraded=false`;
- `runtimeGuard.restartScheduled=false`;
- `system.freeHeap` вище threshold;
- `power.available=true`.

---

## 3. Web UI Verification

Відкрити сторінки:

```text
http://192.168.10.44/
http://192.168.10.44/logs
http://192.168.10.44/config/device
http://192.168.10.44/config/wifi
http://192.168.10.44/config/watchdog
http://192.168.10.44/config/tuya
http://192.168.10.44/config/security
```

### Pass

- сторінки відкриваються без crash;
- `/logs` не викликає stack smashing;
- `/config/tuya` відкривається;
- dashboard не зависає після refresh;
- кнопка `Restart ESP` показується.

---

## 4. HealthCheck Verification

### Дія

Відкрити:

```text
http://192.168.10.44/api/health
```

або:

```text
http://192.168.10.44/api/status
```

### Pass

- `available=true` для доступного target host;
- `responseTime` має реальне значення;
- `consecutiveFails=0` у нормальному стані;
- `sent` збільшується з часом.

---

## 5. Tuya Relay Command Verification

Виконувати тільки якщо безпечно вимкнути контрольоване обладнання.

### OFF

```bash
curl -X POST http://192.168.10.44/api/power/off
```

Очікуваний log:

```text
Connecting to Tuya ...
Tuya connected
Tuya: 3.5 session start sent
Tuya: 3.5 session established
Tuya: 3.5 relay command sent ... state=0
Tuya: 3.5 command ACK received
Tuya: relay state=0
PowerService: power OFF
```

### ON

```bash
curl -X POST http://192.168.10.44/api/power/on
```

Очікуваний log:

```text
Tuya: 3.5 relay command sent ... state=1
Tuya: 3.5 command ACK received
Tuya: relay state=1
PowerService: power ON
```

### Pass

- розетка фізично перемикається;
- ACK приходить;
- `relay state` оновлюється;
- `PowerService` не переходить у error.

---

## 6. Tuya DPQuery Verification

DPQuery поки перевіряємо обережно.

### Дія

Увімкнути в `/config/tuya`:

```json
"statusPollingEnabled": true,
"statusPollingInterval": 60000
```

Зберегти config і перезапустити ESP, якщо UI показав `restartRecommended`.

### Очікуваний log

```text
Tuya: 3.5 status query sent, seq=... dps=1 bytes=...
Tuya: 3.5 packet received, cmd=8 ...
Tuya: relay state=0
```

або:

```text
Tuya: relay state=1
```

### Pass

- DPQuery не викликає crash;
- розетка не disconnect-иться циклічно;
- relay state оновлюється без power command.

### Fail / Recovery

Якщо після DPQuery Tuya socket починає часто disconnect-итися:

```json
"statusPollingEnabled": false
```

Це не блокує power-cycle release, якщо relay OFF/ON path працює стабільно.

---

## 7. Watchdog Restart Cycle

Виконувати тільки в контрольованих умовах.

### Дія

Тимчасово зробити target host недоступним або змінити `watchdog.targetHost` на недоступну адресу.

### Очікуваний log

```text
Watchdog: restart required
Tuya: 3.5 relay command sent ... state=0
PowerService: power OFF
PowerService: restart started
Tuya: 3.5 relay command sent ... state=1
PowerService: power ON
RestartHistory: id=... success reason=watchdog_failure
Watchdog: restart completed, boot delay=...
```

### Pass

- restart cycle завершується;
- restart history фіксує success;
- після `powerOn` Watchdog входить у `bootDelay`;
- restart loop не виникає.

---

## 8. OTA Verification

### Дія

Зібрати й залити OTA:

```bash
pio run -e d1_mini_ota --target upload
```

### Pass

- authentication проходить;
- upload завершується;
- ESP reboot після OTA;
- firmware version відповідає очікуваній;
- RuntimeGuard не перезапускає ESP під час OTA.

---

## 9. Security Verification

Якщо `security.apiAuthEnabled=true`:

### Без token

```bash
curl -X POST http://192.168.10.44/api/power/restart
```

Очікування:

```json
{"ok":false}
```

### З token

```bash
curl -X POST http://192.168.10.44/api/power/restart \
  -H "Authorization: Bearer <api-token>"
```

Очікування:

```json
{"ok":true}
```

### Pass

- state-changing commands захищені;
- query-token fallback не використовується;
- secrets не видно у `/api/config`.

---

## 10. Рішення після hardware verification

### Якщо все пройшло

Переходимо до:

```text
0.9.2-long-run-test
```

### Якщо є bug

Створити bugfix release:

```text
0.9.1.x
```

або:

```text
0.9.2
```

залежно від масштабу.

### Якщо DPQuery нестабільний

Для `1.0.0` залишити:

```json
"statusPollingEnabled": false
```

і вважати DPQuery experimental opt-in, якщо relay OFF/ON path стабільний.
