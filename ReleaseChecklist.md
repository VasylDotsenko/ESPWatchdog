# ESP Watchdog — Release Checklist

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

## Feature Freeze Policy

До `1.0.0` не додаємо нові великі функції.

Дозволено:

- bugfix;
- hardware verification fixes;
- security / stability fixes;
- документація;
- baseline config correction.

Відкласти після `1.0.0`, якщо не блокує реліз:

- average RTT;
- availability history;
- restart history persistent storage;
- full Tuya `3.4` support;
- advanced dashboard analytics.

---

## 0.9.1 — Hardware Verification

### Boot / Storage / Config

- [ ] cold boot проходить без crash;
- [ ] LittleFS монтується;
- [ ] `/config.json` читається;
- [ ] при відсутньому config створюється default config;
- [ ] Serial monitor працює на `74880`;
- [ ] startup logs після NTP sync мають real-time timestamp.

### WiFi / Setup Portal

- [ ] підключення до домашньої WiFi-мережі стабільне;
- [ ] при порожньому `wifi.ssid` стартує `ESP-Watchdog-Setup`;
- [ ] setup portal відкривається на `192.168.4.1`;
- [ ] у setup mode Watchdog / Power-cycle не виконуються;
- [ ] після збереження WiFi config та reboot пристрій підключається до домашньої мережі.
- [ ] якщо домашня WiFi-мережа недоступна, але `wifi.ssid` заданий, AP працює як rescue portal;
- [ ] через 5 хвилин rescue AP вимикається і пристрій повторно пробує STA-підключення;
- [ ] якщо WiFi не відновився протягом 20 хвилин, ESP виконує контрольований restart;
- [ ] якщо `wifi.ssid` порожній, AP не перезавантажує ESP і лишається доступним для першого налаштування.

### HealthCheck

- [ ] TCP/SSH HealthCheck перевіряє `watchdog.targetHost:watchdog.targetPort`;
- [ ] online state визначається коректно;
- [ ] offline state визначається після `watchdog.failCount`;
- [ ] false-positive restart не виникає при одиничній втраті перевірки.

### Watchdog / Power

- [ ] Watchdog trigger створює restart request;
- [ ] PowerService виконує power OFF;
- [ ] PowerService виконує power ON після `powerOffTime`;
- [ ] restart history фіксує success;
- [ ] `bootDelay` cooldown блокує повторний restart одразу після power-cycle;
- [ ] `maxRestartPerDay` блокує restart-loop.

### Tuya LAN `3.5`

- [ ] Tuya LAN socket підключається on-demand;
- [ ] session negotiation `START/RESP/FINISH` проходить успішно;
- [ ] relay OFF command працює;
- [ ] relay ON command працює;
- [ ] command ACK обробляється без warning;
- [ ] `statusPollingEnabled=false` не створює зайвого polling traffic;
- [ ] при `statusPollingEnabled=true` відправляється `DPQueryNew`;
- [ ] після DPQuery приходить status packet;
- [ ] `Tuya: relay state=0/1` оновлюється без power command;
- [ ] якщо DPQuery провокує disconnect, `statusPollingEnabled` лишається disabled для release baseline.

### Web UI / API

- [ ] `/` dashboard відкривається швидко;
- [ ] `/logs` відкривається без stack crash;
- [ ] `/config/device` відкривається;
- [ ] `/config/wifi` відкривається;
- [ ] `/config/watchdog` відкривається;
- [ ] `/config/tuya` відкривається;
- [ ] `/config/security` відкривається;
- [ ] `GET /api/status` працює;
- [ ] `GET /api/diagnostics` працює;
- [ ] `GET /api/logs` працює;
- [ ] `POST /api/config` зберігає config;
- [ ] `POST /api/system/restart` повертає HTTP response до reboot.

### Security

- [ ] state-changing API без token блокується, якщо `apiAuthEnabled=true`;
- [ ] `Authorization: Bearer <token>` працює;
- [ ] query-token fallback відсутній;
- [ ] `wifi.password`, `tuya.localKey`, `security.apiToken` не віддаються відкрито через API;
- [ ] setup portal не блокується auth-перевіркою під час first boot.

### OTA

- [ ] OTAService стартує після WiFi connect;
- [ ] OTA upload працює через `d1_mini_ota`;
- [ ] OTA auth відповідає `security.apiToken`;
- [ ] RuntimeGuard не робить recovery restart під час OTA upload.

---

## 0.9.2 — Long-run Test

Основні документи:

- `LongRunTest.md`;
- `LongRunReport.md`.

- [ ] 24h runtime без unexpected reset;
- [ ] 48h runtime без деградації Web UI;
- [ ] 7d runtime без критичного heap drop;
- [ ] `freeHeap` не падає нижче RuntimeGuard threshold;
- [ ] heap fragmentation лишається нижче RuntimeGuard threshold;
- [ ] WiFi reconnect count не росте аномально;
- [ ] HealthCheck lost count не росте аномально;
- [ ] Watchdog не входить у restart loop;
- [ ] Tuya command path лишається доступним після idle-period;
- [ ] dashboard відкривається після 24h+ runtime;
- [ ] logs page відкривається після 24h+ runtime.

---

## Release Decision

`1.0.0` можна випускати, якщо:

- hardware verification пройдено;
- long-run test пройдено;
- немає crash / stack smashing / WDT reset у нормальному сценарії;
- Tuya power-cycle hardware-verified;
- baseline config перевірений;
- README / Changelog / Roadmap / ProjectStatus актуальні.
