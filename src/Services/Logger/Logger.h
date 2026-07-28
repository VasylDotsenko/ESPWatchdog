#pragma once

#include <Arduino.h>
#include <Print.h>
#include <stdarg.h>

#include "LogLevel.h"

//=============================================================================
// Logger
//=============================================================================
//
// Production Logger
//
// Features
// --------
// • Error
// • Warning
// • Info
// • Debug
// • Verbose
//
// Supports:
//
//   Log.info("Text");
//   Log.info(F("Text"));
//
// Output:
//
//   [000001234][INFO ] WiFi connected
//
//=============================================================================

class Logger
{
public:

    static constexpr size_t BUFFER_SIZE = 128;

public:

    Logger() = default;

    //---------------------------------------------------------------------
    // Initialization
    //---------------------------------------------------------------------

    bool begin(
        uint32_t baudRate = 115200,
        LogLevel level = LogLevel::Info);

    //---------------------------------------------------------------------
    // Configuration
    //---------------------------------------------------------------------

    void setLevel(LogLevel level);

    [[nodiscard]]
    LogLevel level() const;

    //---------------------------------------------------------------------
    // Output
    //---------------------------------------------------------------------

    void setOutput(Print& output);

    //---------------------------------------------------------------------
    // Error
    //---------------------------------------------------------------------

    void error(
        const char* format,
        ...);

    void error(
        const __FlashStringHelper* format,
        ...);

    //---------------------------------------------------------------------
    // Warning
    //---------------------------------------------------------------------

    void warning(
        const char* format,
        ...);

    void warning(
        const __FlashStringHelper* format,
        ...);

    //---------------------------------------------------------------------
    // Info
    //---------------------------------------------------------------------

    void info(
        const char* format,
        ...);

    void info(
        const __FlashStringHelper* format,
        ...);

    //---------------------------------------------------------------------
    // Debug
    //---------------------------------------------------------------------

    void debug(
        const char* format,
        ...);

    void debug(
        const __FlashStringHelper* format,
        ...);

    //---------------------------------------------------------------------
    // Verbose
    //---------------------------------------------------------------------

    void verbose(
        const char* format,
        ...);

    void verbose(
        const __FlashStringHelper* format,
        ...);

private:

    void print(
        LogLevel level,
        const char* prefix,
        const char* format,
        va_list args);

    void print(
        LogLevel level,
        const char* prefix,
        const __FlashStringHelper* format,
        va_list args);

private:

    Print* m_output = nullptr;

    LogLevel m_level = LogLevel::Info;
};

//=============================================================================

extern Logger Log;