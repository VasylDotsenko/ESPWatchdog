#pragma once

#include <Arduino.h>
#include <stdarg.h>

#include "LogLevel.h"

struct LogEntry
{
    static constexpr size_t MESSAGE_LENGTH = 120;

    uint32_t timestamp = 0;
    LogLevel level = LogLevel::Info;
    char levelText[8] {};
    char message[MESSAGE_LENGTH] {};
};

class Logger
{
public:
    static constexpr uint8_t LOG_CAPACITY = 32;

    bool begin(
        uint32_t baudRate = 115200,
        LogLevel level = LogLevel::Info);

    void setLevel(LogLevel level);

    LogLevel level() const;

    void error(const char* format, ...);
    void error(const __FlashStringHelper* format, ...);

    void warning(const char* format, ...);
    void warning(const __FlashStringHelper* format, ...);

    void info(const char* format, ...);
    void info(const __FlashStringHelper* format, ...);

    void debug(const char* format, ...);
    void debug(const __FlashStringHelper* format, ...);

    void verbose(const char* format, ...);
    void verbose(const __FlashStringHelper* format, ...);

    uint8_t entries(
        LogEntry* output,
        uint8_t capacity) const;

    uint8_t count() const;

    bool entry(
        uint8_t position,
        LogEntry& output) const;

    void clear();

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

    void writeLine(
        LogLevel level,
        const char* prefix,
        const char* message);

    void append(
        LogLevel level,
        const char* prefix,
        const char* message);

private:
    LogLevel m_level = LogLevel::Info;

    LogEntry m_entries[LOG_CAPACITY] {};

    uint8_t m_head = 0;

    uint8_t m_count = 0;
};

extern Logger Log;
