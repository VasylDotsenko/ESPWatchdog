#pragma once

#include <Arduino.h>
#include <ESP8266WiFi.h>

extern "C"
{
#include <ping.h>
}

#include "../Common/INetworkSession.h"

// Asynchronous ICMP Echo session backed by the ESP8266 lwIP ping API.
class IcmpSession final : public INetworkSession
{
public:
    bool begin() override;

    bool start(
        const char* host,
        uint16_t port = 0,
        uint32_t timeout = DEFAULT_TIMEOUT_MS) override;

    void loop() override;

    bool running() const override;

    bool finished() const override;

    const NetworkResult& result() const override;

    const char* name() const override;

private:
    enum class State : uint8_t
    {
        Idle,
        Running,
        Finished
    };

    static constexpr uint32_t DEFAULT_TIMEOUT_MS = 1000;
    static constexpr uint32_t DEFAULT_PING_COUNT = 1;
    static constexpr uint32_t MIN_TIMEOUT_MS = 1000;
    static constexpr size_t HOST_BUFFER_SIZE = 64;

    void reset();
    bool resolveHost(const char* host);
    void onReceive(ping_resp* response);
    void complete(
        bool success,
        NetworkResult::Status status,
        ping_resp* response);

    static void recvCallback(void* option, void* response);
    static void sentCallback(void* option, void* response);

    ping_option m_options {};
    IPAddress m_address;
    char m_host[HOST_BUFFER_SIZE] {};
    uint32_t m_timeout = DEFAULT_TIMEOUT_MS;
    State m_state = State::Idle;
    NetworkResult m_result;
};
