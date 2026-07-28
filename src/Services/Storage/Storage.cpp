#include "Services/Storage/Storage.h"

#include "Services/Logger/Logger.h"

StorageService Storage;

bool StorageService::begin()
{
    if (LittleFS.begin())
    {
        Log.info("Storage: LittleFS mounted");
        return true;
    }

    Log.warning("Storage: LittleFS mount failed; formatting");

    if (!LittleFS.format())
    {
        Log.error("Storage: LittleFS format failed");
        return false;
    }

    if (!LittleFS.begin())
    {
        Log.error("Storage: LittleFS mount failed after format");
        return false;
    }

    Log.info("Storage: LittleFS formatted and mounted");
    return true;
}

bool StorageService::exists(const char* path) const
{
    return path != nullptr && LittleFS.exists(path);
}

bool StorageService::remove(const char* path)
{
    if (path == nullptr)
    {
        Log.error("Storage: remove called with null path");
        return false;
    }

    if (!LittleFS.exists(path))
    {
        return true;
    }

    if (!LittleFS.remove(path))
    {
        Log.error("Storage: cannot remove %s", path);
        return false;
    }

    return true;
}

File StorageService::open(const char* path, const char* mode)
{
    if (path == nullptr || mode == nullptr)
    {
        Log.error("Storage: open called with invalid arguments");
        return File();
    }

    return LittleFS.open(path, mode);
}

bool StorageService::readText(const char* path, String& text) const
{
    if (path == nullptr)
    {
        Log.error("Storage: readText called with null path");
        return false;
    }

    File file = LittleFS.open(path, "r");

    if (!file)
    {
        Log.warning("Storage: cannot open %s for reading", path);
        return false;
    }

    text = file.readString();
    file.close();
    return true;
}

bool StorageService::writeText(const char* path, const String& text)
{
    if (path == nullptr)
    {
        Log.error("Storage: writeText called with null path");
        return false;
    }

    File file = LittleFS.open(path, "w");

    if (!file)
    {
        Log.error("Storage: cannot open %s for writing", path);
        return false;
    }

    const size_t written = file.print(text);
    file.close();

    if (written != text.length())
    {
        Log.error("Storage: incomplete write to %s", path);
        return false;
    }

    return true;
}

bool StorageService::readJson(const char* path, JsonDocument& doc) const
{
    if (path == nullptr)
    {
        Log.error("Storage: readJson called with null path");
        return false;
    }

    File file = LittleFS.open(path, "r");

    if (!file)
    {
        Log.warning("Storage: JSON file not found: %s", path);
        return false;
    }

    const DeserializationError error = deserializeJson(doc, file);
    file.close();

    if (error)
    {
        Log.error("Storage: JSON parse error in %s: %s", path, error.c_str());
        return false;
    }

    return true;
}

bool StorageService::writeJson(const char* path, const JsonDocument& doc)
{
    if (path == nullptr)
    {
        Log.error("Storage: writeJson called with null path");
        return false;
    }

    File file = LittleFS.open(path, "w");

    if (!file)
    {
        Log.error("Storage: cannot open JSON file for writing: %s", path);
        return false;
    }

    const size_t written = serializeJsonPretty(doc, file);
    file.close();

    if (written == 0)
    {
        Log.error("Storage: JSON write failed: %s", path);
        return false;
    }

    return true;
}

size_t StorageService::fileSize(const char* path) const
{
    if (path == nullptr)
    {
        return 0;
    }

    File file = LittleFS.open(path, "r");

    if (!file)
    {
        return 0;
    }

    const size_t size = file.size();
    file.close();
    return size;
}
