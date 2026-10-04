#!/usr/bin/env python3
"""Local, read-only diagnostics monitor for ESP Watchdog.

Run this script on a computer connected to the same LAN as the device.
It makes no state-changing requests to the ESP.
"""

from __future__ import annotations

import argparse
import fcntl
import json
import os
import subprocess
import sys
import time
from datetime import datetime
from pathlib import Path
from typing import Any
from urllib.error import URLError
from urllib.request import urlopen


def now_text() -> str:
    return datetime.now().astimezone().strftime("%Y-%m-%d %H:%M:%S %Z")


def notify(title: str, message: str) -> None:
    """Show a macOS notification without invoking a shell."""
    try:
        subprocess.run(
            [
                "osascript",
                "-e",
                f'display notification {json.dumps(message)} with title {json.dumps(title)}',
            ],
            check=False,
            stdout=subprocess.DEVNULL,
            stderr=subprocess.DEVNULL,
        )
    except OSError:
        pass


def get_int(data: dict[str, Any], *path: str) -> int:
    value: Any = data
    for part in path:
        if not isinstance(value, dict):
            return 0
        value = value.get(part, 0)
    return int(value) if isinstance(value, (int, float)) else 0


def get_bool(data: dict[str, Any], *path: str) -> bool:
    value: Any = data
    for part in path:
        if not isinstance(value, dict):
            return False
        value = value.get(part, False)
    return bool(value)


def fetch(url: str, timeout: int) -> dict[str, Any]:
    with urlopen(url, timeout=timeout) as response:
        return json.loads(response.read().decode("utf-8"))


def add_transition_alert(active_conditions: set[str], key: str,
                         condition: bool, message: str,
                         alerts: list[str]) -> None:
    if condition:
        if key not in active_conditions:
            active_conditions.add(key)
            alerts.append(message)
        return

    active_conditions.discard(key)


def evaluate(current: dict[str, Any], previous: dict[str, Any] | None,
             active_conditions: set[str]) -> list[str]:
    alerts: list[str] = []

    add_transition_alert(
        active_conditions,
        "exception_reset",
        get_bool(current, "crashInfo", "exception"),
        "ESP rebooted after an Exception",
        alerts)

    add_transition_alert(
        active_conditions,
        "runtime_degraded",
        get_bool(current, "runtimeGuard", "degraded"),
        "RuntimeGuard reports degraded runtime",
        alerts)

    add_transition_alert(
        active_conditions,
        "restart_scheduled",
        get_bool(current, "runtimeGuard", "restartScheduled"),
        "RuntimeGuard scheduled an ESP restart",
        alerts)

    minimum_heap = get_int(current, "runtimeGuard", "minFreeHeapSeen")
    add_transition_alert(
        active_conditions,
        "minimum_heap_low",
        minimum_heap < 8000,
        f"Minimum heap below 8000 B: {minimum_heap} B",
        alerts)

    system_fragmentation = get_int(current, "system", "heapFragmentation")
    guard_fragmentation = get_int(current, "runtimeGuard", "heapFragmentation")
    fragmentation = max(system_fragmentation, guard_fragmentation)
    add_transition_alert(
        active_conditions,
        "heap_fragmentation_high",
        fragmentation >= 60,
        f"Heap fragmentation is high: {fragmentation}%",
        alerts)

    if previous is not None:
        old_heap = get_int(previous, "runtimeGuard", "freeHeap")
        current_heap = get_int(current, "runtimeGuard", "freeHeap")
        if old_heap > 0 and old_heap - current_heap > 1000:
            alerts.append(f"RuntimeGuard heap dropped {old_heap - current_heap} B within one hour")

        for section, field in (("watchdog", "restartCount"),
                               ("power", "errorCount"),
                               ("tuya", "errorCount")):
            old_value = get_int(previous, section, field)
            new_value = get_int(current, section, field)
            if new_value > old_value:
                alerts.append(f"New {section}.{field}: {old_value} → {new_value}")

    return alerts


def concise(data: dict[str, Any]) -> str:
    return (
        f"heap={get_int(data, 'runtimeGuard', 'freeHeap')} B, "
        f"min={get_int(data, 'runtimeGuard', 'minFreeHeapSeen')} B, "
        f"frag={get_int(data, 'runtimeGuard', 'heapFragmentation')}%, "
        f"drop={get_int(data, 'runtimeGuard', 'heapDropFromBoot')} B, "
        f"uptime={get_int(data, 'system', 'uptimeSeconds')} s"
    )


def main() -> int:
    parser = argparse.ArgumentParser(description="Monitor ESP Watchdog diagnostics locally.")
    parser.add_argument("--url", default="http://192.168.10.44/api/diagnostics")
    parser.add_argument("--hours", type=float, default=24.0)
    parser.add_argument("--interval", type=int, default=3600, help="Seconds between requests.")
    parser.add_argument("--timeout", type=int, default=10)
    parser.add_argument("--output", default="logs/diagnostics-monitor.jsonl")
    parser.add_argument("--no-notifications", action="store_true")
    args = parser.parse_args()

    if args.hours <= 0 or args.interval <= 0:
        parser.error("--hours and --interval must be positive")

    output = Path(args.output)
    output.parent.mkdir(parents=True, exist_ok=True)

    lock_path = output.parent / "diagnostics-monitor.lock"
    lock_file = lock_path.open("w", encoding="utf-8")
    try:
        fcntl.flock(lock_file.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
    except BlockingIOError:
        print(
            f"Another monitor already owns {lock_path}; refusing to start a duplicate.",
            file=sys.stderr,
            flush=True)
        return 2

    lock_file.write(str(os.getpid()))
    lock_file.flush()

    started_at = time.time()
    deadline = started_at + args.hours * 3600
    next_check_at = started_at
    previous: dict[str, Any] | None = None
    active_conditions: set[str] = set()
    failures = 0
    successful_checks = 0
    alert_count = 0
    lowest_heap: int | None = None
    highest_fragmentation = 0

    print(f"[{now_text()}] Monitoring {args.url} for {args.hours:g} hour(s)", flush=True)
    print(f"[{now_text()}] Log file: {output.resolve()}", flush=True)

    while True:
        now = time.time()
        if now < next_check_at:
            time.sleep(next_check_at - now)

        record: dict[str, Any] = {"checkedAt": now_text()}

        try:
            current = fetch(args.url, args.timeout)
            failures = 0
            active_conditions.discard("endpoint_unavailable")
            successful_checks += 1
            alerts = evaluate(current, previous, active_conditions)
            record.update({"ok": True, "diagnostics": current, "alerts": alerts})
            current_heap = get_int(current, "runtimeGuard", "freeHeap")
            fragmentation = max(
                get_int(current, "system", "heapFragmentation"),
                get_int(current, "runtimeGuard", "heapFragmentation"))
            lowest_heap = current_heap if lowest_heap is None else min(lowest_heap, current_heap)
            highest_fragmentation = max(highest_fragmentation, fragmentation)
            print(f"[{record['checkedAt']}] OK    {concise(current)}", flush=True)
            previous = current
        except (URLError, TimeoutError, json.JSONDecodeError, OSError) as error:
            failures += 1
            alerts = []
            add_transition_alert(
                active_conditions,
                "endpoint_unavailable",
                failures >= 2,
                "Diagnostics endpoint is unavailable for two consecutive checks",
                alerts)
            record.update({"ok": False, "error": str(error), "alerts": alerts})
            print(f"[{record['checkedAt']}] ERROR {error}", file=sys.stderr, flush=True)

        if alerts:
            alert_count += len(alerts)
            message = "; ".join(alerts)
            print(f"[{record['checkedAt']}] ALERT {message}", file=sys.stderr, flush=True)
            if not args.no_notifications:
                notify("ESP Watchdog alert", message)

        with output.open("a", encoding="utf-8") as log_file:
            log_file.write(json.dumps(record, ensure_ascii=False) + "\n")

        if time.time() >= deadline:
            break

        next_check_at += args.interval
        while next_check_at <= time.time():
            next_check_at += args.interval

    summary = {
        "finishedAt": now_text(),
        "successfulChecks": successful_checks,
        "alerts": alert_count,
        "lowestRuntimeGuardHeap": lowest_heap,
        "highestFragmentation": highest_fragmentation,
        "output": str(output.resolve()),
    }
    print(
        f"[{summary['finishedAt']}] Finished: "
        f"{json.dumps(summary, ensure_ascii=False)}",
        flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
