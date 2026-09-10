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

- [ ] Dashboard opens normally
- [ ] Logs page opens normally
- [ ] Config pages open normally
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

### T+33h

Date/time:

```text
2026-09-10
```

Diagnostics:

```json
{"ok":true,"level":"warn","system":{"freeHeap":10992,"heapFragmentation":25,"uptimeSeconds":119557,"resetReason":"Software/System restart","heapWarning":true,"fragmentationWarning":false},"network":{"connected":true,"rssi":-67,"quality":66,"reconnectCount":0,"warning":false},"health":{"available":true,"running":false,"responseTime":5,"sent":23889,"lost":23,"consecutiveFails":0,"warning":false},"watchdog":{"enabled":true,"restartPending":false,"lockedOut":false,"cooldown":false,"restartCount":1,"warning":false},"power":{"available":true,"restartInProgress":false,"restartCount":1,"errorCount":0,"warning":false},"tuya":{"connected":false,"relayState":true,"reconnectCount":0,"commandCount":2,"errorCount":0,"connectedAt":22693953,"lastDisconnectedAt":22734321,"lastCommandAt":22704195,"lastPacketAt":22714296,"lastErrorAt":0,"sessionStartCount":1,"sessionEstablishedCount":1,"sessionFailureCount":0},"runtimeGuard":{"enabled":true,"degraded":false,"restartScheduled":false,"freeHeap":11480,"heapFragmentation":28,"heapAtBoot":19640,"minFreeHeapSeen":11096,"heapDropFromBoot":8160,"maxHeapFragmentationSeen":35,"minFreeHeap":8000,"maxHeapFragmentation":60,"lastCheckAt":119534691,"degradedSince":0,"restartAt":0}}
```

Result:

- [x] Runtime is alive after 33h+
- [x] Dashboard/API diagnostics responds
- [x] WiFi remains connected
- [x] HealthCheck remains online
- [x] Watchdog is not pending restart
- [x] PowerService is available
- [x] RuntimeGuard is not degraded
- [ ] System level is `warn` due to heap warning

Notes:

```text
Uptime is about 33h 12m 37s. HealthCheck packet loss is low: 23 / 23889 ≈ 0.096%.
Main observation: system.freeHeap=10992 and heapWarning=true, but RuntimeGuard still reports degraded=false and restartScheduled=false.
Continue toward T+48h. Watch heap trend, minFreeHeapSeen and dashboard responsiveness.
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
