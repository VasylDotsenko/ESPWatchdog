#pragma once

#include <Arduino.h>

#include "Models/SystemData.h"

struct SystemStatusFirmware
{
    char version[Model::VERSION_LENGTH] {};
    char sdkVersion[Model::SDK_VERSION_LENGTH] {};
    char coreVersion[Model::CORE_VERSION_LENGTH] {};
    char buildDate[Model::BUILD_DATE_LENGTH] {};
    char buildTime[Model::BUILD_TIME_LENGTH] {};
    char resetReason[Model::RESET_REASON_LENGTH] {};
};

struct SystemStatusUptime
{
    uint64_t milliseconds = 0;
    uint64_t seconds = 0;

    uint32_t days = 0;
    uint8_t hours = 0;
    uint8_t minutes = 0;
    uint8_t secondsPart = 0;
};

struct SystemStatusMemory
{
    uint32_t freeHeap = 0;
    uint32_t heapFragmentation = 0;
    uint32_t sketchSize = 0;
    uint32_t freeSketchSpace = 0;
    uint32_t flashChipSize = 0;
};

struct SystemStatusCpu
{
    uint32_t chipId = 0;
    uint32_t frequencyMHz = 0;
};

struct SystemStatusData
{
    SystemStatusFirmware firmware;
    SystemStatusUptime uptime;
    SystemStatusMemory memory;
    SystemStatusCpu cpu;
};
