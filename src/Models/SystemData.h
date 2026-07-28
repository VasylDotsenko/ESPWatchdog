#pragma once

#include <Arduino.h>

#include "Common.h"

//=============================================================================
// System Uptime
//=============================================================================

struct SystemUptime
{
    uint64_t milliseconds = 0;

    uint64_t seconds = 0;

    uint32_t days = 0;

    uint8_t hours = 0;

    uint8_t minutes = 0;

    uint8_t secondsPart = 0;
};

//=============================================================================
// System Memory
//=============================================================================

struct SystemMemory
{
    uint32_t freeHeap = 0;

    uint32_t heapFragmentation = 0;

    uint32_t sketchSize = 0;

    uint32_t freeSketchSpace = 0;

    uint32_t flashChipSize = 0;
};

//=============================================================================
// System CPU
//=============================================================================

struct SystemCpu
{
    uint32_t chipId = 0;

    uint32_t frequencyMHz = 0;
};

//=============================================================================
// System Firmware
//=============================================================================

struct SystemFirmware
{
    char version[Model::VERSION_LENGTH] {};

    char sdkVersion[Model::SDK_VERSION_LENGTH] {};

    char coreVersion[Model::CORE_VERSION_LENGTH] {};

    char buildDate[Model::BUILD_DATE_LENGTH] {};

    char buildTime[Model::BUILD_TIME_LENGTH] {};

    char resetReason[Model::RESET_REASON_LENGTH] {};
};

//=============================================================================
// System Data
//=============================================================================

struct SystemData
{
    SystemUptime uptime;

    SystemMemory memory;

    SystemCpu cpu;

    SystemFirmware firmware;
};