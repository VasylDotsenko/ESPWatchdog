#pragma once

#include "Services/HealthCheck/IHealthCheckProvider.h"

// Returns the TCP provider without exposing ESP8266 SDK implementation
// details to Application.cpp or other high-level services.
IHealthCheckProvider& tcpHealthCheckProvider();
