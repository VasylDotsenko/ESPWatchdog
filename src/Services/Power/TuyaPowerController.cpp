#include "TuyaPowerController.h"

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
    return TuyaLan.connected();
}

bool TuyaPowerController::powerOn()
{
    if (!available())
    {
        Log.warning("TuyaPowerController: Tuya LAN unavailable");
        return false;
    }

    return TuyaLan.relayOn();
}

bool TuyaPowerController::powerOff()
{
    if (!available())
    {
        Log.warning("TuyaPowerController: Tuya LAN unavailable");
        return false;
    }

    return TuyaLan.relayOff();
}

const char* TuyaPowerController::name() const
{
    return "TuyaPowerController";
}
