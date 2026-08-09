#include "Services/SystemInfo/SystemInfo.h"

#include <ESP.h>
#include <cstring>

#include "Core/BuildInfo.h"

namespace
{
void copyText(char* destination, size_t destinationSize, const char* source)
{
    if (destinationSize == 0)
    {
        return;
    }

    if (source == nullptr)
    {
        destination[0] = '\0';
        return;
    }

    std::strncpy(destination, source, destinationSize - 1);
    destination[destinationSize - 1] = '\0';
}
}

SystemInfoService System;

bool SystemInfoService::begin()
{
    m_data = SystemData {};
    m_uptimeMs = 0;
    m_lastMillis = millis();

    update();

    m_updateTimer.start(
        UPDATE_INTERVAL_MS,
        TimerMode::Periodic);

    return true;
}

void SystemInfoService::loop()
{
    if (m_updateTimer.expired())
    {
        update();
    }
}

const SystemData& SystemInfoService::data() const
{
    return m_data;
}

SystemStatusData SystemInfoService::status() const
{
    SystemStatusData status;

    std::memcpy(
        status.firmware.version,
        m_data.firmware.version,
        sizeof(status.firmware.version));

    std::memcpy(
        status.firmware.sdkVersion,
        m_data.firmware.sdkVersion,
        sizeof(status.firmware.sdkVersion));

    std::memcpy(
        status.firmware.coreVersion,
        m_data.firmware.coreVersion,
        sizeof(status.firmware.coreVersion));

    std::memcpy(
        status.firmware.buildDate,
        m_data.firmware.buildDate,
        sizeof(status.firmware.buildDate));

    std::memcpy(
        status.firmware.buildTime,
        m_data.firmware.buildTime,
        sizeof(status.firmware.buildTime));

    std::memcpy(
        status.firmware.resetReason,
        m_data.firmware.resetReason,
        sizeof(status.firmware.resetReason));

    status.uptime.milliseconds = m_data.uptime.milliseconds;
    status.uptime.seconds = m_data.uptime.seconds;
    status.uptime.days = m_data.uptime.days;
    status.uptime.hours = m_data.uptime.hours;
    status.uptime.minutes = m_data.uptime.minutes;
    status.uptime.secondsPart = m_data.uptime.secondsPart;

    status.memory.freeHeap = m_data.memory.freeHeap;
    status.memory.heapFragmentation = m_data.memory.heapFragmentation;
    status.memory.sketchSize = m_data.memory.sketchSize;
    status.memory.freeSketchSpace = m_data.memory.freeSketchSpace;
    status.memory.flashChipSize = m_data.memory.flashChipSize;

    status.cpu.chipId = m_data.cpu.chipId;
    status.cpu.frequencyMHz = m_data.cpu.frequencyMHz;

    return status;
}

void SystemInfoService::update()
{
    updateUptime();
    updateMemory();
    updateCpu();
    updateFirmware();
}

void SystemInfoService::updateUptime()
{
    const uint32_t now = millis();
    m_uptimeMs += static_cast<uint32_t>(now - m_lastMillis);
    m_lastMillis = now;

    SystemUptime& uptime = m_data.uptime;
    uptime.milliseconds = m_uptimeMs;
    uptime.seconds = m_uptimeMs / 1000ULL;

    uint64_t remainingSeconds = uptime.seconds;
    uptime.days = static_cast<uint32_t>(remainingSeconds / 86400ULL);
    remainingSeconds %= 86400ULL;
    uptime.hours = static_cast<uint8_t>(remainingSeconds / 3600ULL);
    remainingSeconds %= 3600ULL;
    uptime.minutes = static_cast<uint8_t>(remainingSeconds / 60ULL);
    uptime.secondsPart = static_cast<uint8_t>(remainingSeconds % 60ULL);
}

void SystemInfoService::updateMemory()
{
    SystemMemory& memory = m_data.memory;
    memory.freeHeap = ESP.getFreeHeap();
    memory.heapFragmentation = ESP.getHeapFragmentation();
    memory.sketchSize = ESP.getSketchSize();
    memory.freeSketchSpace = ESP.getFreeSketchSpace();
    memory.flashChipSize = ESP.getFlashChipRealSize();
}

void SystemInfoService::updateCpu()
{
    m_data.cpu.frequencyMHz = ESP.getCpuFreqMHz();
    m_data.cpu.chipId = ESP.getChipId();
}

void SystemInfoService::updateFirmware()
{
    SystemFirmware& firmware = m_data.firmware;

    copyText(firmware.version, sizeof(firmware.version), BuildInfo::version());
    copyText(firmware.buildDate, sizeof(firmware.buildDate), BuildInfo::buildDate());
    copyText(firmware.buildTime, sizeof(firmware.buildTime), BuildInfo::buildTime());
    copyText(firmware.sdkVersion, sizeof(firmware.sdkVersion), ESP.getSdkVersion());

#if defined(ARDUINO_ESP8266_GIT_VER)
    const String coreVersion = ESP.getCoreVersion();
    copyText(firmware.coreVersion, sizeof(firmware.coreVersion), coreVersion.c_str());
#else
    copyText(firmware.coreVersion, sizeof(firmware.coreVersion), "Unknown");
#endif

    const String resetReason = ESP.getResetReason();
    copyText(firmware.resetReason, sizeof(firmware.resetReason), resetReason.c_str());
}
