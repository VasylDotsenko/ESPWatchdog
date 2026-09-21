# ESP Watchdog — Long-run Report

Release promoted from baseline:

```text
0.9.3-crashinfo
```

Production release:

```text
1.0.0
```

Start date:

```text
2026-09-14
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

### T+105h

Date/time:

```text
2026-09-13
```

Diagnostics:

```json
{"ok":true,"level":"warn","system":{"freeHeap":11376,"heapFragmentation":27,"uptimeSeconds":380556,"resetReason":"Software/System restart","heapWarning":true,"fragmentationWarning":false},"network":{"connected":true,"rssi":-65,"quality":70,"reconnectCount":0,"warning":false},"health":{"available":true,"running":false,"responseTime":3,"sent":76062,"lost":55,"consecutiveFails":0,"warning":false},"watchdog":{"enabled":true,"restartPending":false,"lockedOut":false,"cooldown":false,"restartCount":2,"warning":false},"power":{"available":true,"restartInProgress":false,"restartCount":2,"errorCount":0,"warning":false},"tuya":{"connected":false,"relayState":true,"reconnectCount":0,"commandCount":4,"errorCount":0,"connectedAt":185338986,"lastDisconnectedAt":185379370,"lastCommandAt":185349232,"lastPacketAt":185359337,"lastErrorAt":0,"sessionStartCount":2,"sessionEstablishedCount":2,"sessionFailureCount":0},"runtimeGuard":{"enabled":true,"degraded":false,"restartScheduled":false,"freeHeap":10936,"heapFragmentation":24,"heapAtBoot":19640,"minFreeHeapSeen":10992,"heapDropFromBoot":8704,"maxHeapFragmentationSeen":36,"minFreeHeap":8000,"maxHeapFragmentation":60,"lastCheckAt":380551304,"degradedSince":0,"restartAt":0}}
```

Result:

- [x] Runtime is alive after 105h+
- [x] Dashboard/API diagnostics responds
- [x] WiFi remains connected
- [x] WiFi reconnect count remains 0
- [x] HealthCheck remains online
- [x] Watchdog is not pending restart
- [x] PowerService is available
- [x] Tuya command path has no recorded errors
- [x] RuntimeGuard is not degraded
- [x] RuntimeGuard restart is not scheduled
- [ ] System level remains `warn` due to heap warning

Notes:

```text
Uptime is about 105h 42m 36s, or about 4.4 days.
HealthCheck packet loss is 55 / 76062 ≈ 0.072%, which is lower than the T+33h ratio.
Heap appears stable: system.freeHeap=11376 vs 10992 at T+33h, runtimeGuard.freeHeap=10936, minFreeHeapSeen=10992, maxHeapFragmentationSeen=36%.
The device remains below warning comfort level but above RuntimeGuard hard thresholds: minFreeHeap=8000, maxHeapFragmentation=60.
Watchdog/Power restart counters increased from 1 to 2, and Tuya commandCount increased from 2 to 4 with errorCount=0. This suggests another controlled power-cycle completed without Tuya errors.
Continue toward T+7d. Main thing to watch: whether minFreeHeapSeen drops below 10 KB or RuntimeGuard enters degraded state.
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

## Exception reset event

Date/time:

```text
2026-09-14
```

Diagnostics after reboot:

```json
{"ok":true,"level":"ok","system":{"freeHeap":12240,"heapFragmentation":29,"uptimeSeconds":79622,"resetReason":"Exception","heapWarning":false,"fragmentationWarning":false},"network":{"connected":true,"rssi":-65,"quality":70,"reconnectCount":2,"warning":false},"health":{"available":true,"running":false,"responseTime":46,"sent":15864,"lost":7,"consecutiveFails":0,"warning":false},"watchdog":{"enabled":true,"restartPending":false,"lockedOut":false,"cooldown":false,"restartCount":0,"warning":false},"power":{"available":true,"restartInProgress":false,"restartCount":0,"errorCount":0,"warning":false},"tuya":{"connected":false,"relayState":false,"reconnectCount":0,"commandCount":0,"errorCount":0,"connectedAt":0,"lastDisconnectedAt":0,"lastCommandAt":0,"lastPacketAt":0,"lastErrorAt":0,"sessionStartCount":0,"sessionEstablishedCount":0,"sessionFailureCount":0},"runtimeGuard":{"enabled":true,"degraded":false,"restartScheduled":false,"freeHeap":10304,"heapFragmentation":20,"heapAtBoot":19640,"minFreeHeapSeen":10784,"heapDropFromBoot":9336,"maxHeapFragmentationSeen":34,"minFreeHeap":8000,"maxHeapFragmentation":60,"lastCheckAt":79589768,"degradedSince":0,"restartAt":0}}
```

Result:

- [x] Device recovered after reboot
- [x] Current diagnostics level is `ok`
- [x] WiFi is connected
- [x] HealthCheck is online
- [x] PowerService is available
- [x] RuntimeGuard is not degraded
- [ ] Long-run test cannot be marked as passed because reset reason is `Exception`
- [ ] Root cause is unknown without exception decoder / serial crash log

Notes:

```text
Previous snapshot was T+105h on 2026-09-13. Current uptime is 79622s ≈ 22h 07m after an unexpected reboot.
The device recovered cleanly, but resetReason="Exception" means the long-run test must be considered interrupted.
Heap after reboot is healthy enough: system.freeHeap=12240, runtimeGuard.freeHeap=10304, minFreeHeapSeen=10784, maxHeapFragmentationSeen=34%.
This does not look like RuntimeGuard-driven recovery: restartScheduled=false, degraded=false.
Need serial crash block / exception decoder output from the reboot event, or persistent crash storage in firmware for future runs.
```

---

## CrashInfo hotfix follow-up

Date/time:

```text
2026-09-14
```

Firmware prepared:

```text
0.9.3-crashinfo
```

Purpose:

- expose `ESP.getResetInfo()` after reboot;
- expose `exceptionReset` flag;
- add compact `crashInfo` object to `/api/diagnostics`;
- preserve full reset info in `/api/status/system`.

Expected diagnostics after the next reboot:

```json
"crashInfo": {
  "exception": true,
  "resetReason": "Exception",
  "resetInfo": "..."
}
```

Verification snapshot:

```json
{"ok":true,"level":"warn","system":{"freeHeap":11152,"heapFragmentation":3,"uptimeSeconds":55,"resetReason":"Software/System restart","heapWarning":true,"fragmentationWarning":false},"crashInfo":{"exception":false,"resetReason":"Software/System restart","resetInfo":"Software/System restart"},"network":{"connected":true,"rssi":-64,"quality":72,"reconnectCount":0,"warning":false},"health":{"available":true,"running":false,"responseTime":2,"sent":11,"lost":0,"consecutiveFails":0,"warning":false},"watchdog":{"enabled":true,"restartPending":false,"lockedOut":false,"cooldown":false,"restartCount":0,"warning":false},"power":{"available":true,"restartInProgress":false,"restartCount":0,"errorCount":0,"warning":false},"tuya":{"connected":false,"relayState":false,"reconnectCount":0,"commandCount":0,"errorCount":0,"connectedAt":0,"lastDisconnectedAt":0,"lastCommandAt":0,"lastPacketAt":0,"lastErrorAt":0,"sessionStartCount":0,"sessionEstablishedCount":0,"sessionFailureCount":0},"runtimeGuard":{"enabled":true,"degraded":false,"restartScheduled":false,"freeHeap":10712,"heapFragmentation":3,"heapAtBoot":19128,"minFreeHeapSeen":19128,"heapDropFromBoot":8416,"maxHeapFragmentationSeen":6,"minFreeHeap":8000,"maxHeapFragmentation":60,"lastCheckAt":3567,"degradedSince":0,"restartAt":0}}
```

Verification result:

- [x] `/api/diagnostics` contains `crashInfo`
- [x] `crashInfo.exception=false` after normal software/system restart
- [x] `crashInfo.resetReason` is populated
- [x] `crashInfo.resetInfo` is populated
- [x] Device boots and reconnects to WiFi
- [x] HealthCheck is online
- [x] RuntimeGuard is not degraded
- [ ] diagnostics level is `warn` because `system.heapWarning=true`

Notes:

```text
The previous 0.9.2 long-run was interrupted by resetReason="Exception".
0.9.3-crashinfo was verified after normal software/system restart: crashInfo is present and exception=false.
Heap warning is still present at boot because system.freeHeap=11152 is below the diagnostics warning threshold of 12000, but fragmentation is low and RuntimeGuard is not degraded.
The next long-run attempt should continue with 0.9.3-crashinfo so any future unexpected reboot contains enough post-reboot crash context for root-cause analysis.
```

---

## 0.9.3-crashinfo long-run snapshot

### T+169h / 7-day validation

Date/time:

```text
2026-09-21
```

Diagnostics:

```json
{"ok":true,"level":"warn","system":{"freeHeap":11536,"heapFragmentation":26,"uptimeSeconds":611407,"resetReason":"Software/System restart","heapWarning":true,"fragmentationWarning":false},"crashInfo":{"exception":false,"resetReason":"Software/System restart","resetInfo":"Software/System restart"},"network":{"connected":true,"rssi":-64,"quality":72,"reconnectCount":2,"warning":false},"health":{"available":true,"running":false,"responseTime":4,"sent":122139,"lost":88,"consecutiveFails":0,"warning":false},"watchdog":{"enabled":true,"restartPending":false,"lockedOut":false,"cooldown":false,"restartCount":3,"warning":false},"power":{"available":true,"restartInProgress":false,"restartCount":3,"errorCount":0,"warning":false},"tuya":{"connected":false,"relayState":true,"reconnectCount":0,"commandCount":6,"errorCount":0,"connectedAt":213923858,"lastDisconnectedAt":213964054,"lastCommandAt":213933919,"lastPacketAt":213944024,"lastErrorAt":0,"sessionStartCount":3,"sessionEstablishedCount":3,"sessionFailureCount":0},"runtimeGuard":{"enabled":true,"degraded":false,"restartScheduled":false,"freeHeap":11312,"heapFragmentation":24,"heapAtBoot":19128,"minFreeHeapSeen":10080,"heapDropFromBoot":7816,"maxHeapFragmentationSeen":36,"minFreeHeap":8000,"maxHeapFragmentation":60,"lastCheckAt":611366628,"degradedSince":0,"restartAt":0}}
```

Result:

- [x] Runtime is alive after 7 days
- [x] No exception reset recorded during this run
- [x] `crashInfo.exception=false`
- [x] WiFi is connected; reconnect count is stable at `2`
- [x] HealthCheck is online; packet loss is `88 / 122139` (about `0.072%`)
- [x] Watchdog and PowerService are not in a restart loop
- [x] Tuya command path has `0` errors and all `3` sessions were established
- [x] RuntimeGuard is not degraded and has no scheduled restart
- [x] Minimum free heap remained above the `8000` byte guard threshold
- [x] Maximum heap fragmentation remained below the `60%` guard threshold
- [ ] Diagnostics level remains `warn` because the informational system heap threshold is `12000` bytes

Notes:

```text
Uptime is 611407 seconds: 7 days, 1 hour, 50 minutes and 7 seconds.
The test satisfies the 7-day long-run criterion for 0.9.3-crashinfo.
The observed minimum free heap is 10080 bytes, which is 2080 bytes above the RuntimeGuard threshold. Heap fragmentation peaked at 36%, 24 percentage points below its threshold.
The system-level heap warning is expected under the current 12000-byte diagnostics threshold and is not a RuntimeGuard degradation or restart condition.
Tuya is intentionally disconnected while idle; its last controlled power cycles completed without recorded command or session failures.
```

### T+91h

Date/time:

```text
2026-09-18
```

Diagnostics:

```json
{"ok":true,"level":"warn","system":{"freeHeap":10904,"heapFragmentation":22,"uptimeSeconds":327781,"resetReason":"Software/System restart","heapWarning":true,"fragmentationWarning":false},"crashInfo":{"exception":false,"resetReason":"Software/System restart","resetInfo":"Software/System restart"},"network":{"connected":true,"rssi":-60,"quality":80,"reconnectCount":2,"warning":false},"health":{"available":true,"running":false,"responseTime":4,"sent":65426,"lost":76,"consecutiveFails":0,"warning":false},"watchdog":{"enabled":true,"restartPending":false,"lockedOut":false,"cooldown":false,"restartCount":3,"warning":false},"power":{"available":true,"restartInProgress":false,"restartCount":3,"errorCount":0,"warning":false},"tuya":{"connected":false,"relayState":true,"reconnectCount":0,"commandCount":6,"errorCount":0,"connectedAt":213923858,"lastDisconnectedAt":213964054,"lastCommandAt":213933919,"lastPacketAt":213944024,"lastErrorAt":0,"sessionStartCount":3,"sessionEstablishedCount":3,"sessionFailureCount":0},"runtimeGuard":{"enabled":true,"degraded":false,"restartScheduled":false,"freeHeap":10352,"heapFragmentation":18,"heapAtBoot":19128,"minFreeHeapSeen":10536,"heapDropFromBoot":8776,"maxHeapFragmentationSeen":35,"minFreeHeap":8000,"maxHeapFragmentation":60,"lastCheckAt":327726780,"degradedSince":0,"restartAt":0}}
```

Result:

- [x] Runtime is alive after 91h+
- [x] CrashInfo is present
- [x] `crashInfo.exception=false`
- [x] Reset reason is normal software/system restart
- [x] WiFi remains connected
- [x] HealthCheck remains online
- [x] Watchdog is not pending restart
- [x] PowerService is available
- [x] Tuya command path has no recorded errors
- [x] RuntimeGuard is not degraded
- [x] RuntimeGuard restart is not scheduled
- [ ] diagnostics level remains `warn` due to heap warning

Notes:

```text
Uptime is about 91h 03m 01s, or about 3.8 days on 0.9.3-crashinfo.
HealthCheck packet loss is 76 / 65426 ≈ 0.116%, still acceptable.
Heap remains in the known yellow zone but above RuntimeGuard threshold: system.freeHeap=10904, runtimeGuard.freeHeap=10352, minFreeHeapSeen=10536, minFreeHeap threshold=8000.
Fragmentation is not concerning: system.heapFragmentation=22%, runtimeGuard.heapFragmentation=18%, maxHeapFragmentationSeen=35%, threshold=60%.
WiFi reconnectCount=2, but network is currently connected and RSSI/quality improved compared to previous snapshots.
Watchdog/Power restartCount=3 and Tuya commandCount=6 with errorCount=0, sessionEstablishedCount=3 and sessionFailureCount=0. This suggests controlled power-cycle path remains healthy.
Continue toward 7d on 0.9.3-crashinfo. Current state does not indicate memory leak or crash recurrence.
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
PASS — 0.9.3-crashinfo completed the 7-day stability validation; baseline promoted to 1.0.0 Production
```

The preceding `0.9.2` run was interrupted by an Exception reset. The `0.9.3-crashinfo` rerun completed 7 days without a recorded exception or RuntimeGuard recovery.

The release baseline was promoted to `1.0.0 Production` on 21.09.2026. Future observations belong to production monitoring:

- 24h / 48h / 7d checks pass;
- no crash / stack smashing / WDT reset;
- Web UI remains usable;
- OTA remains usable;
- Tuya LAN power-cycle remains usable;
- WiFi AP recovery behaves as expected.
