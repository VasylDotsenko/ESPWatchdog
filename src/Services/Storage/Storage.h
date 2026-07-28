#pragma once

#include <Arduino.h>
#include <LittleFS.h>
#include <ArduinoJson.h>

class StorageService
{
public:

    bool begin();

    bool exists(const char* path) const;

    bool remove(const char* path);

    File open(const char* path, const char* mode);

    bool readText(
        const char* path,
        String& text) const;

    bool writeText(
        const char* path,
        const String& text);

    bool readJson(
        const char* path,
        JsonDocument& doc) const;

    bool writeJson(
        const char* path,
        const JsonDocument& doc);

    size_t fileSize(const char* path) const;
};

extern StorageService Storage;