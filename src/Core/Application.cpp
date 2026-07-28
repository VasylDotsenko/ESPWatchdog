#include "Application.h"

#include "Services/Config/Config.h"
#include "Services/HealthCheck/HealthCheckService.h"
#include "Services/HealthCheck/IcmpHealthCheckProvider.h"
#include "Services/Logger/Logger.h"
#include "Services/Relay/RelayService.h"
#include "Services/Storage/Storage.h"
#include "Services/SystemInfo/SystemInfo.h"
#include "Services/Watchdog/WatchdogService.h"
#include "Services/WiFi/WiFiService.h"

//=============================================================================
// Global instance
//=============================================================================

Application App;

//=============================================================================
// Begin
//=============================================================================

bool Application::begin()
{
    if (!Log.begin(74880))
    {
        return false;
    }

    Log.info("========================================");
    Log.info("ESP Watchdog");
    Log.info("========================================");

    if (!Storage.begin())
    {
        Log.error("Application: Storage initialization failed");
        return false;
    }

    if (!Config.begin())
    {
        Log.error("Application: Config initialization failed");
        return false;
    }

    if (!Network.begin())
    {
        Log.error("Application: Network initialization failed");
        return false;
    }

    if (!System.begin())
    {
        Log.error("Application: System initialization failed");
        return false;
    }

    if (!Relay.begin())
    {
        Log.error("Application: Relay initialization failed");
        return false;
    }

    HealthCheck.setProvider(IcmpProvider);

    if (!HealthCheck.begin())
    {
        Log.error("Application: HealthCheck initialization failed");
        return false;
    }

    if (!Watchdog.begin())
    {
        Log.error("Application: Watchdog initialization failed");
        return false;
    }

    Log.info("Application: started");

    return true;
}

//=============================================================================
// Main loop
//=============================================================================

void Application::loop()
{
    Network.loop();

    System.loop();

    HealthCheck.loop();

    Watchdog.update(HealthCheck.info());

    Watchdog.loop();

    if (Watchdog.restartRequired() &&
        !Relay.restartInProgress())
    {
        Relay.restart(
            Watchdog.data().configuration.powerOffTime);
    }

    Relay.loop();

    if (Relay.restartCompleted())
    {
        Relay.clearRestartCompleted();

        Watchdog.restartCompleted();
    }
}
