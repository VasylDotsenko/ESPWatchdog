#include "Logger.h"

Logger Log;

//=============================================================================
// Initialization
//=============================================================================

bool Logger::begin(
    uint32_t baudRate,
    LogLevel level)
{
    Serial.begin(baudRate);

    m_output = &Serial;

    m_level = level;

    return true;
}

//=============================================================================
// Configuration
//=============================================================================

void Logger::setLevel(LogLevel level)
{
    m_level = level;
}

//-----------------------------------------------------------------------------

LogLevel Logger::level() const
{
    return m_level;
}

//=============================================================================
// Output
//=============================================================================

void Logger::setOutput(Print& output)
{
    m_output = &output;
}

//=============================================================================
// Error
//=============================================================================

void Logger::error(
    const char* format,
    ...)
{
    va_list args;

    va_start(args, format);

    print(
        LogLevel::Error,
        "ERROR",
        format,
        args);

    va_end(args);
}

//-----------------------------------------------------------------------------

void Logger::error(
    const __FlashStringHelper* format,
    ...)
{
    va_list args;

    va_start(args, format);

    print(
        LogLevel::Error,
        "ERROR",
        format,
        args);

    va_end(args);
}

//=============================================================================
// Warning
//=============================================================================

void Logger::warning(
    const char* format,
    ...)
{
    va_list args;

    va_start(args, format);

    print(
        LogLevel::Warning,
        "WARN ",
        format,
        args);

    va_end(args);
}

//-----------------------------------------------------------------------------

void Logger::warning(
    const __FlashStringHelper* format,
    ...)
{
    va_list args;

    va_start(args, format);

    print(
        LogLevel::Warning,
        "WARN ",
        format,
        args);

    va_end(args);
}

//=============================================================================
// Info
//=============================================================================

void Logger::info(
    const char* format,
    ...)
{
    va_list args;

    va_start(args, format);

    print(
        LogLevel::Info,
        "INFO ",
        format,
        args);

    va_end(args);
}

//-----------------------------------------------------------------------------

void Logger::info(
    const __FlashStringHelper* format,
    ...)
{
    va_list args;

    va_start(args, format);

    print(
        LogLevel::Info,
        "INFO ",
        format,
        args);

    va_end(args);
}

//=============================================================================
// Debug
//=============================================================================

void Logger::debug(
    const char* format,
    ...)
{
    va_list args;

    va_start(args, format);

    print(
        LogLevel::Debug,
        "DEBUG",
        format,
        args);

    va_end(args);
}

//-----------------------------------------------------------------------------

void Logger::debug(
    const __FlashStringHelper* format,
    ...)
{
    va_list args;

    va_start(args, format);

    print(
        LogLevel::Debug,
        "DEBUG",
        format,
        args);

    va_end(args);
}

//=============================================================================
// Verbose
//=============================================================================

void Logger::verbose(
    const char* format,
    ...)
{
    va_list args;

    va_start(args, format);

    print(
        LogLevel::Verbose,
        "TRACE",
        format,
        args);

    va_end(args);
}

//-----------------------------------------------------------------------------

void Logger::verbose(
    const __FlashStringHelper* format,
    ...)
{
    va_list args;

    va_start(args, format);

    print(
        LogLevel::Verbose,
        "TRACE",
        format,
        args);

    va_end(args);
}

//=============================================================================
// Print (RAM)
//=============================================================================

void Logger::print(
    LogLevel level,
    const char* prefix,
    const char* format,
    va_list args)
{
    if (m_output == nullptr)
    {
        return;
    }

    if (static_cast<uint8_t>(level) >
        static_cast<uint8_t>(m_level))
    {
        return;
    }

    char buffer[BUFFER_SIZE];

    vsnprintf(
        buffer,
        sizeof(buffer),
        format,
        args);

    m_output->printf(
        "[%010lu][%-5s] %s\r\n",
        millis(),
        prefix,
        buffer);
}

//=============================================================================
// Print (PROGMEM)
//=============================================================================

void Logger::print(
    LogLevel level,
    const char* prefix,
    const __FlashStringHelper* format,
    va_list args)
{
    if (m_output == nullptr)
    {
        return;
    }

    if (static_cast<uint8_t>(level) >
        static_cast<uint8_t>(m_level))
    {
        return;
    }

    char buffer[BUFFER_SIZE];

    vsnprintf_P(
        buffer,
        sizeof(buffer),
        reinterpret_cast<PGM_P>(format),
        args);

    m_output->printf(
        "[%010lu][%-5s] %s\r\n",
        millis(),
        prefix,
        buffer);
}