# ESP Watchdog — Long-run Test

Дата старту: 08.09.2026

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

Підтвердити стабільну unattended роботу ESP Watchdog протягом тривалого часу без ручного перезапуску живлення.

Основні ризики цього етапу:

- деградація Web Dashboard після довгого uptime;
- падіння `freeHeap`;
- зростання `heapFragmentation`;
- зависання WiFi recovery в AP mode;
- нестабільність Tuya LAN після idle-period;
- restart-loop після Watchdog / RuntimeGuard recovery.

---

## Контрольні точки

### 1 година

- dashboard відкривається швидко;
- `/api/diagnostics` повертає JSON без timeout;
- `freeHeap` не нижче RuntimeGuard threshold;
- `runtimeGuard.degraded=false`;
- `network.connected=true`;
- `power.available=true`.

### 24 години

- Web Dashboard доступний;
- `/logs` відкривається без crash;
- `heapFragmentation` не перевищує RuntimeGuard threshold;
- `HealthCheck.lost` не росте аномально;
- `Watchdog` не входить у restart-loop;
- Tuya relay command path доступний після idle-period.

### 48 годин

- немає ручного power-cycle ESP;
- немає `Stack smashing detected`;
- немає `Exception`;
- немає WDT reset у нормальному сценарії;
- Web UI не деградує до непридатного стану.

### 7 днів

- firmware залишається керованим через Web UI;
- OTA залишається доступним після довгого uptime;
- RuntimeGuard не виконує зайві restart;
- WiFi reconnect / AP recovery працює передбачувано;
- Tuya LAN power-cycle проходить успішно.

---

## Щоденна перевірка

Відкрити:

```text
http://192.168.10.44/api/diagnostics
```

Зафіксувати:

- `system.freeHeap`;
- `system.heapFragmentation`;
- `system.uptimeSeconds`;
- `network.connected`;
- `network.reconnectCount`;
- `health.available`;
- `health.sent`;
- `health.lost`;
- `watchdog.restartCount`;
- `power.restartCount`;
- `power.errorCount`;
- `runtimeGuard.degraded`;
- `runtimeGuard.restartScheduled`.

---

## WiFi recovery test

Один раз під час long-run етапу перевірити сценарій втрати домашньої WiFi-мережі:

1. Залишити валідний `wifi.ssid` у `config.json`.
2. Тимчасово вимкнути домашній WiFi або заблокувати ESP на роутері.
3. Переконатися, що ESP переходить у `ESP-Watchdog-Setup`.
4. Переконатися, що через 5 хвилин AP вимикається і ESP пробує STA-підключення.
5. Якщо WiFi не повернувся протягом 20 хвилин, ESP має виконати контрольований restart.

Pass:

- ESP не зависає назавжди в AP mode;
- після повернення WiFi пристрій підключається до домашньої мережі;
- Watchdog / Power-cycle не запускаються під час setup portal.

---

## Tuya idle-period test

Після 24h+ uptime виконати ручну команду:

```text
POST /api/power/restart
```

Очікування:

- Tuya LAN підключається on-demand;
- session negotiation проходить успішно;
- relay OFF виконується;
- relay ON виконується після `powerOffTime`;
- `RestartHistory` фіксує success;
- `PowerService` не переходить у permanent error.

---

## Критерії проходження етапу

Етап `0.9.2-long-run-test` вважається пройденим, якщо:

- 24h runtime пройдено без crash;
- 48h runtime пройдено без деградації Web UI;
- 7d runtime пройдено без критичного heap drop;
- WiFi AP recovery підтверджено;
- Tuya command path підтверджено після idle-period;
- RuntimeGuard не створює restart-loop;
- документація та baseline config актуальні.

