#include "Logger.h"

#include <cstdio>
#include <cstring>
#include <time.h>

Logger Log;

bool Logger::begin(
    uint32_t baudRate,
    LogLevel level)
{
    Serial.begin(baudRate);

    m_level = level;

    return true;
}

void Logger::setLevel(LogLevel level)
{
    m_level = level;
}

LogLevel Logger::level() const
{
    return m_level;
}

void Logger::error(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    print(LogLevel::Error, "ERROR", format, args);
    va_end(args);
}

void Logger::error(const __FlashStringHelper* format, ...)
{
    va_list args;
    va_start(args, format);
    print(LogLevel::Error, "ERROR", format, args);
    va_end(args);
}

void Logger::warning(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    print(LogLevel::Warning, "WARN ", format, args);
    va_end(args);
}

void Logger::warning(const __FlashStringHelper* format, ...)
{
    va_list args;
    va_start(args, format);
    print(LogLevel::Warning, "WARN ", format, args);
    va_end(args);
}

void Logger::info(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    print(LogLevel::Info, "INFO ", format, args);
    va_end(args);
}

void Logger::info(const __FlashStringHelper* format, ...)
{
    va_list args;
    va_start(args, format);
    print(LogLevel::Info, "INFO ", format, args);
    va_end(args);
}

void Logger::debug(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    print(LogLevel::Debug, "DEBUG", format, args);
    va_end(args);
}

void Logger::debug(const __FlashStringHelper* format, ...)
{
    va_list args;
    va_start(args, format);
    print(LogLevel::Debug, "DEBUG", format, args);
    va_end(args);
}

void Logger::verbose(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    print(LogLevel::Verbose, "TRACE", format, args);
    va_end(args);
}

void Logger::verbose(const __FlashStringHelper* format, ...)
{
    va_list args;
    va_start(args, format);
    print(LogLevel::Verbose, "TRACE", format, args);
    va_end(args);
}

uint8_t Logger::entries(
    LogEntry* output,
    uint8_t capacity) const
{
    if (output == nullptr ||
        capacity == 0)
    {
        return 0;
    }

    const uint8_t count =
        m_count < capacity
            ? m_count
            : capacity;

    const uint8_t start =
        m_count < LOG_CAPACITY
            ? 0
            : m_head;

    for (uint8_t i = 0; i < count; ++i)
    {
        const uint8_t index =
            static_cast<uint8_t>(
                (start + i) % LOG_CAPACITY);

        output[i] = m_entries[index];
    }

    return count;
}

uint8_t Logger::count() const
{
    return m_count;
}

bool Logger::entry(
    uint8_t position,
    LogEntry& output) const
{
    if (position >= m_count)
    {
        return false;
    }

    const uint8_t start =
        m_count < LOG_CAPACITY
            ? 0
            : m_head;

    const uint8_t index =
        static_cast<uint8_t>(
            (start + position) % LOG_CAPACITY);

    output = m_entries[index];

    return true;
}

void Logger::clear()
{
    m_head = 0;
    m_count = 0;

    for (uint8_t i = 0; i < LOG_CAPACITY; ++i)
    {
        m_entries[i] = LogEntry {};
    }
}

void Logger::synchronizeWallTime()
{
    const time_t now =
        time(nullptr);

    if (now < static_cast<time_t>(1600000000UL))
    {
        return;
    }

    const uint32_t nowMillis =
        millis();

    for (uint8_t i = 0; i < LOG_CAPACITY; ++i)
    {
        LogEntry& entry =
            m_entries[i];

        if (entry.message[0] == '\0' ||
            entry.wallTime != 0)
        {
            continue;
        }

        const uint32_t elapsedMs =
            static_cast<uint32_t>(
                nowMillis - entry.timestamp);

        const time_t entryTime =
            now - static_cast<time_t>(elapsedMs / 1000UL);

        if (!formatWallTimeAt(
                entryTime,
                entry.wallTimeText,
                sizeof(entry.wallTimeText)))
        {
            entry.wallTimeText[0] = '\0';
            continue;
        }

        entry.wallTime =
            static_cast<uint32_t>(entryTime);
    }
}

void Logger::print(
    LogLevel level,
    const char* prefix,
    const char* format,
    va_list args)
{
    if (static_cast<uint8_t>(level) >
        static_cast<uint8_t>(m_level))
    {
        return;
    }

    char buffer[LogEntry::MESSAGE_LENGTH] {};

    vsnprintf(
        buffer,
        sizeof(buffer),
        format,
        args);

    writeLine(level, prefix, buffer);
}

void Logger::print(
    LogLevel level,
    const char* prefix,
    const __FlashStringHelper* format,
    va_list args)
{
    if (static_cast<uint8_t>(level) >
        static_cast<uint8_t>(m_level))
    {
        return;
    }

    char buffer[LogEntry::MESSAGE_LENGTH] {};

    vsnprintf_P(
        buffer,
        sizeof(buffer),
        reinterpret_cast<PGM_P>(format),
        args);

    writeLine(level, prefix, buffer);
}

void Logger::writeLine(
    LogLevel level,
    const char* prefix,
    const char* message)
{
    const uint32_t now =
        millis();

    char timestamp[LogEntry::WALL_TIME_LENGTH] {};
    uint32_t epoch = 0;

    if (!formatWallTime(
            timestamp,
            sizeof(timestamp),
            epoch))
    {
        snprintf(
            timestamp,
            sizeof(timestamp),
            "%010lu",
            static_cast<unsigned long>(now));
    }

    Serial.print('[');
    Serial.print(timestamp);
    Serial.print("][");
    Serial.print(prefix != nullptr ? prefix : "INFO ");
    Serial.print("] ");
    Serial.println(message != nullptr ? message : "");

    append(level, prefix, message);
}

void Logger::append(
    LogLevel level,
    const char* prefix,
    const char* message)
{
    LogEntry& entry =
        m_entries[m_head];

    entry.timestamp =
        millis();
    entry.wallTime = 0;
    entry.level =
        level;

    uint32_t epoch = 0;

    if (formatWallTime(
            entry.wallTimeText,
            sizeof(entry.wallTimeText),
            epoch))
    {
        entry.wallTime =
            epoch;
    }
    else
    {
        entry.wallTimeText[0] =
            '\0';
    }

    strncpy(
        entry.levelText,
        prefix != nullptr ? prefix : "INFO ",
        sizeof(entry.levelText) - 1);

    entry.levelText[sizeof(entry.levelText) - 1] = '\0';

    strncpy(
        entry.message,
        message != nullptr ? message : "",
        sizeof(entry.message) - 1);

    entry.message[sizeof(entry.message) - 1] = '\0';

    m_head =
        static_cast<uint8_t>(
            (m_head + 1) % LOG_CAPACITY);

    if (m_count < LOG_CAPACITY)
    {
        ++m_count;
    }
}

bool Logger::formatWallTime(
    char* output,
    size_t outputSize,
    uint32_t& epoch)
{
    epoch = 0;

    if (output == nullptr ||
        outputSize == 0)
    {
        return false;
    }

    output[0] = '\0';

    const time_t now =
        time(nullptr);

    if (now < static_cast<time_t>(1600000000UL))
    {
        return false;
    }

    if (!formatWallTimeAt(
            now,
            output,
            outputSize))
    {
        output[0] = '\0';
        return false;
    }

    epoch =
        static_cast<uint32_t>(now);

    return true;
}

bool Logger::formatWallTimeAt(
    time_t timestamp,
    char* output,
    size_t outputSize)
{
    if (output == nullptr ||
        outputSize == 0)
    {
        return false;
    }

    output[0] = '\0';

    if (timestamp < static_cast<time_t>(1600000000UL))
    {
        return false;
    }

    struct tm localTime {};

    if (localtime_r(
            &timestamp,
            &localTime) == nullptr)
    {
        return false;
    }

    return strftime(
        output,
        outputSize,
        "%Y-%m-%d %H:%M:%S",
        &localTime) != 0;
}
