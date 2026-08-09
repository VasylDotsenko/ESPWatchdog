#pragma once

#include <Arduino.h>

enum class LogLevel : uint8_t
{
    Error = 0,
    Warning,
    Info,
    Debug,
    Verbose
};
