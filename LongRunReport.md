# ESP Watchdog — Long-run Report

Baseline:

```text
0.9.2-long-run-test
```

Start date:

```text
2026-09-09
```

Device:

```text
ESP8266 / WeMos D1 mini
```

Primary dashboard:

```text
http://192.168.10.44/
```

Diagnostics endpoint:

```text
http://192.168.10.44/api/diagnostics
```

---

## Snapshot command

```bash
curl -s http://192.168.10.44/api/diagnostics
```

If `jq` is installed:

```bash
curl -s http://192.168.10.44/api/diagnostics | jq
```

---

## Observation log

### T+1h

Date/time:

```text

```

Diagnostics:

```json

```

Result:

- [х] Dashboard opens normally
- [х] Logs page opens normally
- [х] Config pages open normally
- [ ] `freeHeap` is above RuntimeGuard threshold
- [ ] `heapFragmentation` is below RuntimeGuard threshold
- [ ] `runtimeGuard.degraded=false`
- [ ] `runtimeGuard.restartScheduled=false`
- [ ] `network.connected=true`
- [ ] `health.available=true`
- [ ] `power.available=true`

Notes:

```text

```

---

### T+24h

Date/time:

```text

```

Diagnostics:

```json

```

Result:

- [ ] Dashboard opens normally
- [ ] Logs page opens without crash
- [ ] OTA remains available
- [ ] Tuya command path works after idle-period
- [ ] No unexpected ESP restart
- [ ] No restart-loop
- [ ] No visible Web UI slowdown

Notes:

```text

```

---

### T+48h

Date/time:

```text

```

Diagnostics:

```json

```

Result:

- [ ] Dashboard opens normally
- [ ] Config pages save correctly
- [ ] Logs page opens without crash
- [ ] No `Stack smashing detected`
- [ ] No ESP exception
- [ ] No WDT reset in normal scenario
- [ ] Heap remains stable

Notes:

```text

```

---

### T+7d

Date/time:

```text

```

Diagnostics:

```json

```

Result:

- [ ] Firmware remains controllable via Web UI
- [ ] OTA remains available
- [ ] RuntimeGuard does not create restart-loop
- [ ] WiFi reconnect remains stable
- [ ] WiFi AP recovery verified
- [ ] Tuya LAN power-cycle verified
- [ ] Device did not require manual power-cycle

Notes:

```text

```

---

## WiFi AP recovery test

Date/time:

```text

```

Scenario:

- [ ] Configured `wifi.ssid` is present
- [ ] Home WiFi temporarily unavailable
- [ ] ESP enters `ESP-Watchdog-Setup`
- [ ] AP rescue window lasts about 5 minutes
- [ ] ESP retries configured WiFi
- [ ] If WiFi remains unavailable, ESP restarts after about 20 minutes
- [ ] If WiFi returns, ESP connects back to home network

Serial / logs:

```text

```

Result:

```text
PASS / FAIL
```

Notes:

```text

```

---

## Tuya idle-period test

Date/time:

```text

```

Command:

```bash
curl -X POST http://192.168.10.44/api/power/restart
```

Expected:

- [ ] Tuya connects on-demand
- [ ] Tuya `3.5` session established
- [ ] Relay OFF confirmed
- [ ] Relay ON confirmed
- [ ] RestartHistory records success
- [ ] PowerService does not enter permanent error

Serial / logs:

```text

```

Result:

```text
PASS / FAIL
```

Notes:

```text

```

---

## Final decision

```text
PENDING
```

Release candidate can move toward `1.0.0` if:

- 24h / 48h / 7d checks pass;
- no crash / stack smashing / WDT reset;
- Web UI remains usable;
- OTA remains usable;
- Tuya LAN power-cycle remains usable;
- WiFi AP recovery behaves as expected.

