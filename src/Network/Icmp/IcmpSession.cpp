#include "IcmpSession.h"

#include <cstring>

#include "Services/Logger/Logger.h"

bool IcmpSession::begin()
{
    reset();
    return true;
}

bool IcmpSession::start(
    const char* host,
    uint16_t,
    uint32_t timeout)
{
    if (running())
    {
        Log.warning(F("ICMP: session already running"));
        return false;
    }

    reset();

    if (host == nullptr || host[0] == '\0')
    {
        Log.error(F("ICMP: host is empty"));
        return false;
    }

    std::strncpy(m_host, host, sizeof(m_host) - 1);
    m_host[sizeof(m_host) - 1] = '\0';

    m_timeout = timeout < MIN_TIMEOUT_MS ? MIN_TIMEOUT_MS : timeout;

    if (!resolveHost(m_host))
    {
        return false;
    }

    std::memset(&m_options, 0, sizeof(m_options));
    m_options.count = DEFAULT_PING_COUNT;
    m_options.ip = static_cast<uint32_t>(m_address);
    m_options.coarse_time = (m_timeout + 999U) / 1000U;
    m_options.reverse = this;
    m_options.recv_function = reinterpret_cast<ping_recv_function>(&IcmpSession::recvCallback);
    m_options.sent_function = reinterpret_cast<ping_sent_function>(&IcmpSession::sentCallback);

    // State must be set before ping_start(): the SDK owns the operation after
    // this call and completes it through recvCallback().
    m_state = State::Running;

    if (!ping_start(&m_options))
    {
        m_state = State::Idle;
        m_result.status = NetworkResult::Status::Error;
        Log.error(F("ICMP: ping_start() failed"));
        return false;
    }

    Log.verbose(F("ICMP: started host=%s"), m_host);
    return true;
}

void IcmpSession::loop()
{
    // lwIP drives the session through callbacks; no polling is required.
}

bool IcmpSession::running() const
{
    return m_state == State::Running;
}

bool IcmpSession::finished() const
{
    return m_state == State::Finished;
}

const NetworkResult& IcmpSession::result() const
{
    return m_result;
}

const char* IcmpSession::name() const
{
    return "ICMP";
}

void IcmpSession::reset()
{
    std::memset(&m_options, 0, sizeof(m_options));
    std::memset(m_host, 0, sizeof(m_host));
    m_address = IPAddress();
    m_timeout = DEFAULT_TIMEOUT_MS;
    m_state = State::Idle;
    m_result.reset();
}

bool IcmpSession::resolveHost(const char* host)
{
    if (!WiFi.hostByName(host, m_address))
    {
        m_result.status = NetworkResult::Status::DnsFailed;
        Log.warning(F("ICMP: DNS lookup failed for %s"), host);
        return false;
    }

    return true;
}

void IcmpSession::recvCallback(void* option, void* response)
{
    auto* pingOption = static_cast<ping_option*>(option);

    if (pingOption == nullptr || pingOption->reverse == nullptr)
    {
        return;
    }

    auto* self = static_cast<IcmpSession*>(pingOption->reverse);
    self->onReceive(static_cast<ping_resp*>(response));
}

void IcmpSession::sentCallback(void*, void*)
{
    // The ESP8266 SDK prints raw ping statistics when sent_function is null.
    // Registering this no-op callback keeps Serial output under Logger control.
}

void IcmpSession::onReceive(ping_resp* response)
{
    if (response == nullptr)
    {
        complete(false, NetworkResult::Status::Error, nullptr);
        return;
    }

    complete(
        response->ping_err == 0,
        response->ping_err == 0
            ? NetworkResult::Status::Success
            : NetworkResult::Status::Timeout,
        response);
}

void IcmpSession::complete(
    bool success,
    NetworkResult::Status status,
    ping_resp* response)
{
    if (!running())
    {
        return;
    }

    m_result.reset();
    m_result.success = success;
    m_result.status = status;
    m_result.packetsSent = DEFAULT_PING_COUNT;

    if (success && response != nullptr)
    {
        m_result.responseTime = response->resp_time;
        m_result.packetsReceived = DEFAULT_PING_COUNT;
    }
    else
    {
        m_result.packetsLost = DEFAULT_PING_COUNT;
    }

    m_state = State::Finished;

    Log.verbose(
        F("ICMP: host=%s status=%u rtt=%lu ms"),
        m_host,
        static_cast<uint8_t>(m_result.status),
        static_cast<unsigned long>(m_result.responseTime));
}
