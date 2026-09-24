#pragma once

#include "Models/RestartHistoryData.h"

// Persists the bounded restart-history ring buffer. It is intentionally
// separate from Config: restart telemetry must never block config loading.
class RestartHistoryStorage final
{
public:
    bool load(RestartHistoryData& history) const;

    bool save(const RestartHistoryData& history) const;

private:
    static constexpr const char* FILE_PATH = "/restart-history.json";
    static constexpr uint8_t FORMAT_VERSION = 1;
};
