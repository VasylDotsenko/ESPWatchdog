#pragma once

#include <Arduino.h>

//=============================================================================
// LogLevel
//=============================================================================
//
// Logger verbosity level.
//
// Lower value = higher priority.
//
//=============================================================================

enum class LogLevel : uint8_t
{
    Error = 0,

    Warning,

    Info,

    Debug,

    Verbose
};