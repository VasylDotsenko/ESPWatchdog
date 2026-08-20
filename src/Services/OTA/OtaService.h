#pragma once

#include <Arduino.h>

class OtaService
{
public:
    bool begin();

    void loop();

    bool active() const;

    bool updating() const;

private:
    bool start();

    void setUpdating(
        bool updating);

private:
    bool m_started = false;

    bool m_updating = false;
};

extern OtaService OTA;
