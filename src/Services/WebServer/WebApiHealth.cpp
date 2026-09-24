#include "WebApiHealth.h"

#include "Services/HealthCheck/HealthCheckService.h"
#include "WebApiResponse.h"

void WebApiHealth::handleClearHistory(
    ESP8266WebServer& server)
{
    HealthCheck.clearAvailabilityHistory();

    WebApiResponse::sendJson(
        server,
        200,
        "{\"ok\":true,\"command\":\"availability_history_clear\"}");
}
