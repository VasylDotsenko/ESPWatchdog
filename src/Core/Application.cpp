#include "Application.h"

#include "Services/Config/Config.h"
#include "Services/HealthCheck/HealthCheckService.h"
#include "Services/HealthCheck/TcpHealthCheckProvider.h"
#include "Services/Logger/Logger.h"
#include "Services/Power/PowerService.h"
#include "Services/Power/TuyaPowerController.h"
#include "Services/Storage/Storage.h"
#include "Services/SystemInfo/SystemInfo.h"
#include "Services/Tuya/TuyaService.h"
#include "Services/Watchdog/WatchdogService.h"
#include "Services/WebServer/WebServerService.h"
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

    if (!TuyaLan.begin())
    {
        Log.error("Application: Tuya initialization failed");
        return false;
    }

    Power.setController(TuyaPower);

    if (!Power.begin())
    {
        Log.error("Application: Power initialization failed");
        return false;
    }

    HealthCheck.setProvider(TcpProvider);

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

    if (!WebServer.begin())
    {
        Log.error("Application: WebServer initialization failed");
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

    if (Network.setupMode())
    {
        WebServer.loop();
        return;
    }

    TuyaLan.loop();

    HealthCheck.loop();

    Watchdog.update(HealthCheck.info());

    Watchdog.loop();

    if (Watchdog.restartRequired() &&
        !Power.restartInProgress())
    {
        Power.restart(
            Watchdog.data().configuration.powerOffTime);
    }

    Power.loop();

    if (Power.restartCompleted())
    {
        Power.clearRestartCompleted();

        Watchdog.restartCompleted();
    }

    WebServer.loop();
}

//=============================================================================
// Status snapshot
//=============================================================================

ApiStatusData Application::status() const
{
    ApiStatusData status;

    status.system = System.status();
    status.network = Network.status();
    status.health = HealthCheck.status();
    status.watchdog = Watchdog.status();
    status.power = Power.status();

    return status;
}
