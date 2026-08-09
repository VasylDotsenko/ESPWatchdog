#include "WebApiResponse.h"

void WebApiResponse::applyHeaders(
    ESP8266WebServer& server)
{
    server.sendHeader(
        "Access-Control-Allow-Origin",
        "*");

    server.sendHeader(
        "Access-Control-Allow-Methods",
        "GET, POST, OPTIONS");

    server.sendHeader(
        "Access-Control-Allow-Headers",
        "Content-Type, Authorization");

    server.sendHeader(
        "Cache-Control",
        "no-store");
}

void WebApiResponse::sendJson(
    ESP8266WebServer& server,
    int statusCode,
    const char* json)
{
    applyHeaders(server);

    server.send(
        statusCode,
        "application/json",
        json != nullptr ? json : "{}");
}

void WebApiResponse::sendNoContent(
    ESP8266WebServer& server)
{
    applyHeaders(server);

    server.send(
        204,
        "application/json",
        "");
}
