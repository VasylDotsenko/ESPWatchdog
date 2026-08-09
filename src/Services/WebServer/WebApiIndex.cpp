#include "WebApiIndex.h"

#include "WebApiResponse.h"

namespace
{
    constexpr const char* API_INDEX_JSON =
        "{"
        "\"name\":\"ESP Watchdog API\","
        "\"version\":1,"
        "\"endpoints\":["
        "{\"method\":\"GET\",\"path\":\"/\",\"description\":\"dashboard\"},"
        "{\"method\":\"GET\",\"path\":\"/dashboard\",\"description\":\"dashboard\"},"
        "{\"method\":\"GET\",\"path\":\"/api\",\"description\":\"api index\"},"
        "{\"method\":\"GET\",\"path\":\"/api/status\",\"description\":\"aggregate status\"},"
        "{\"method\":\"GET\",\"path\":\"/api/system\",\"description\":\"system status\"},"
        "{\"method\":\"GET\",\"path\":\"/api/network\",\"description\":\"network status\"},"
        "{\"method\":\"GET\",\"path\":\"/api/health\",\"description\":\"health status\"},"
        "{\"method\":\"GET\",\"path\":\"/api/watchdog\",\"description\":\"watchdog status\"},"
        "{\"method\":\"GET\",\"path\":\"/api/power\",\"description\":\"power status\"},"
        "{\"method\":\"GET\",\"path\":\"/api/config\",\"description\":\"runtime configuration\"},"
        "{\"method\":\"GET\",\"path\":\"/api/logs\",\"description\":\"runtime logs\"},"
        "{\"method\":\"POST\",\"path\":\"/api/system/restart\",\"description\":\"restart ESP\"},"
        "{\"method\":\"POST\",\"path\":\"/api/config\",\"description\":\"update configuration\"},"
        "{\"method\":\"POST\",\"path\":\"/api/power/on\",\"description\":\"turn power on\"},"
        "{\"method\":\"POST\",\"path\":\"/api/power/off\",\"description\":\"turn power off\"},"
        "{\"method\":\"POST\",\"path\":\"/api/power/restart\",\"description\":\"restart power output\"},"
        "{\"method\":\"GET\",\"path\":\"/config/device\",\"description\":\"device configuration page\"},"
        "{\"method\":\"GET\",\"path\":\"/config/wifi\",\"description\":\"wifi configuration page\"},"
        "{\"method\":\"GET\",\"path\":\"/config/watchdog\",\"description\":\"watchdog configuration page\"},"
        "{\"method\":\"GET\",\"path\":\"/config/relay\",\"description\":\"relay configuration page\"},"
        "{\"method\":\"GET\",\"path\":\"/config/tuya\",\"description\":\"tuya configuration page\"},"
        "{\"method\":\"GET\",\"path\":\"/config/security\",\"description\":\"security configuration page\"},"
        "{\"method\":\"GET\",\"path\":\"/logs\",\"description\":\"runtime logs page\"},"
        "{\"method\":\"GET\",\"path\":\"/health\",\"description\":\"liveness\"}"
        "]"
        "}";

}

void WebApiIndex::handleGet(
    ESP8266WebServer& server)
{
    WebApiResponse::sendJson(
        server,
        200,
        API_INDEX_JSON);
}
