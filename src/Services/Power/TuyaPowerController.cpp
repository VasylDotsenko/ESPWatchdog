#include "TuyaPowerController.h"

#include <ESP8266WiFi.h>
#include <cstring>

#include "Services/Config/Config.h"
#include "Services/Logger/Logger.h"
#include "Services/Tuya/TuyaService.h"

TuyaPowerController TuyaPower;

bool TuyaPowerController::begin()
{
    Log.info("TuyaPowerController: started");

    return true;
}

bool TuyaPowerController::available() const
{
    const auto& cfg = Config.data().tuya;

    return WiFi.isConnected() &&
           cfg.ipAddress[0] != '\0' &&
           cfg.port != 0 &&
           cfg.deviceId[0] != '\0' &&
           cfg.localKey[0] != '\0' &&
           cfg.relayDps != 0;
}

bool TuyaPowerController::powerOn()
{
    if (!available())
    {
        Log.warning("TuyaPowerController: not available");
        return false;
    }

    return TuyaLan.relayOn();
}

bool TuyaPowerController::powerOff()
{
    if (!available())
    {
        Log.warning("TuyaPowerController: not available");
        return false;
    }

    return TuyaLan.relayOff();
}

const char* TuyaPowerController::name() const
{
    return "TuyaPowerController";
}
