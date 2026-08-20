#include "RuntimeGuardService.h"

#include <ESP.h>

#include "Services/Logger/Logger.h"
#include "Services/Power/PowerService.h"
#include "Services/SystemInfo/SystemInfo.h"
#include "Services/WiFi/WiFiService.h"

RuntimeGuardService RuntimeGuard;

bool RuntimeGuardService::begin()
{
    m_status = RuntimeGuardStatus {};
    m_status.enabled = true;
    m_status.minFreeHeap = MIN_FREE_HEAP_BYTES;
    m_status.maxHeapFragmentation = MAX_HEAP_FRAGMENTATION;
    m_lastCheck = 0;

    Log.info(
        "RuntimeGuard: started, minHeap=%lu bytes, maxFrag=%u%%",
        static_cast<unsigned long>(MIN_FREE_HEAP_BYTES),
        MAX_HEAP_FRAGMENTATION);

    return true;
}

void RuntimeGuardService::loop()
{
    const uint32_t now = millis();

    if (m_status.restartScheduled)
    {
        if (static_cast<int32_t>(now - m_status.restartAt) >= 0)
        {
            delay(ESP_RESTART_SETTLE_MS);
            yield();
            ESP.restart();
        }

        return;
    }

    if (m_lastCheck != 0 &&
        static_cast<uint32_t>(now - m_lastCheck) < CHECK_INTERVAL_MS)
    {
        return;
    }

    m_lastCheck = now;

    check();
}

RuntimeGuardStatus RuntimeGuardService::status() const
{
    return m_status;
}

void RuntimeGuardService::check()
{
    if (!m_status.enabled)
    {
        return;
    }

    if (Network.setupMode() ||
        Power.restartInProgress())
    {
        resetDegradedState();
        return;
    }

    const SystemStatusData system =
        System.status();

    m_status.freeHeap =
        system.memory.freeHeap;

    m_status.heapFragmentation =
        system.memory.heapFragmentation;

    if (system.uptime.milliseconds < MIN_UPTIME_BEFORE_RECOVERY_MS)
    {
        resetDegradedState();
        return;
    }

    const bool heapLow =
        system.memory.freeHeap < MIN_FREE_HEAP_BYTES;

    const bool heapFragmented =
        system.memory.heapFragmentation > MAX_HEAP_FRAGMENTATION;

    if (!heapLow &&
        !heapFragmented)
    {
        resetDegradedState();
        return;
    }

    const uint32_t now = millis();

    if (!m_status.degraded)
    {
        m_status.degraded = true;
        m_status.degradedSince = now;

        Log.warning(
            "RuntimeGuard: runtime degraded, heap=%lu bytes, frag=%lu%%",
            static_cast<unsigned long>(system.memory.freeHeap),
            static_cast<unsigned long>(system.memory.heapFragmentation));

        return;
    }

    if (static_cast<uint32_t>(now - m_status.degradedSince) >=
        DEGRADED_DURATION_MS)
    {
        scheduleRestart(
            heapLow
                ? "low_heap"
                : "heap_fragmentation");
    }
}

void RuntimeGuardService::scheduleRestart(
    const char* reason)
{
    if (m_status.restartScheduled)
    {
        return;
    }

    m_status.restartScheduled = true;
    m_status.restartAt = millis() + RESTART_DELAY_MS;

    Log.error(
        "RuntimeGuard: ESP restart scheduled, reason=%s heap=%lu frag=%lu%% delay=%lu ms",
        reason != nullptr ? reason : "runtime_degraded",
        static_cast<unsigned long>(m_status.freeHeap),
        static_cast<unsigned long>(m_status.heapFragmentation),
        static_cast<unsigned long>(RESTART_DELAY_MS));
}

void RuntimeGuardService::resetDegradedState()
{
    m_status.degraded = false;
    m_status.degradedSince = 0;
}
