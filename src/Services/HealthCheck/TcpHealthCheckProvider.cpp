#include "TcpHealthCheckProvider.h"

#include <cstring>

extern "C"
{
#include <user_interface.h>
}

#include "Services/Config/Config.h"
#include "Services/Logger/Logger.h"

TcpHealthCheckProvider TcpProvider;

//=============================================================================

IHealthCheckProvider& tcpHealthCheckProvider()
{
    return TcpProvider;
}

//=============================================================================

bool TcpHealthCheckProvider::begin()
{
    reset();
    return true;
}

//=============================================================================

bool TcpHealthCheckProvider::start(
    const char* host,
    uint32_t timeoutMs)
{
    if (m_running)
    {
        Log.warning("TCP provider: previous check is still running");
        return false;
    }

    reset();

    if (host == nullptr ||
        host[0] == '\0')
    {
        m_result.success = false;
        m_result.status = HealthCheckStatus::Error;
        m_finished = true;
        return true;
    }

    if (wifi_station_get_connect_status() != STATION_GOT_IP)
    {
        m_result.success = false;
        m_result.status = HealthCheckStatus::NetworkUnavailable;
        m_finished = true;
        return true;
    }

    const auto& watchdog = Config.data().watchdog;

    if (watchdog.targetPort == 0)
    {
        m_result.success = false;
        m_result.status = HealthCheckStatus::Error;
        m_finished = true;
        return true;
    }

    m_running = true;
    m_startedAt = millis();
    m_timeoutMs = timeoutMs;

    std::strncpy(
        m_host,
        host,
        sizeof(m_host) - 1);
    m_host[sizeof(m_host) - 1] = '\0';

    m_connection = {};
    m_tcp = {};

    m_connection.type = ESPCONN_TCP;
    m_connection.state = ESPCONN_NONE;
    m_connection.proto.tcp = &m_tcp;
    m_connection.reverse = this;

    m_tcp.local_port = espconn_port();
    m_tcp.remote_port = watchdog.targetPort;

    espconn_regist_connectcb(
        &m_connection,
        &TcpHealthCheckProvider::onConnected);
    espconn_regist_reconcb(
        &m_connection,
        &TcpHealthCheckProvider::onReconnect);
    espconn_regist_disconcb(
        &m_connection,
        &TcpHealthCheckProvider::onDisconnected);

    const err_t dnsResult = espconn_gethostbyname(
        &m_connection,
        m_host,
        &m_resolvedAddress,
        &TcpHealthCheckProvider::onDnsFound);

    if (dnsResult == ESPCONN_OK)
    {
        beginConnection(m_resolvedAddress);
        return true;
    }

    if (dnsResult == ESPCONN_INPROGRESS)
    {
        m_dnsPending = true;
        return true;
    }

    completeFailure(HealthCheckStatus::DnsFailed);
    return true;
}

//=============================================================================

void TcpHealthCheckProvider::loop()
{
    if (!m_running)
    {
        return;
    }

    if (static_cast<uint32_t>(millis() - m_startedAt) >= m_timeoutMs)
    {
        completeFailure(HealthCheckStatus::Timeout);
    }
}

//=============================================================================

bool TcpHealthCheckProvider::running() const
{
    return m_running;
}

//=============================================================================

bool TcpHealthCheckProvider::finished() const
{
    return m_finished;
}

//=============================================================================

void TcpHealthCheckProvider::cancel()
{
    if (!m_running)
    {
        return;
    }

    m_running = false;
    m_finished = true;
    m_dnsPending = false;

    m_result.success = false;
    m_result.status = HealthCheckStatus::Cancelled;
    m_result.responseTime = 0;

    abortConnection();
}

//=============================================================================

const HealthCheckResult& TcpHealthCheckProvider::result() const
{
    return m_result;
}

//=============================================================================

TcpHealthCheckProvider* TcpHealthCheckProvider::providerFromCallback(
    void* argument)
{
    auto* connection = static_cast<espconn*>(argument);

    if (connection == nullptr ||
        connection->reverse == nullptr)
    {
        return nullptr;
    }

    return static_cast<TcpHealthCheckProvider*>(connection->reverse);
}

//=============================================================================

void TcpHealthCheckProvider::onConnected(void* argument)
{
    TcpHealthCheckProvider* provider = providerFromCallback(argument);

    if (provider != nullptr)
    {
        provider->handleConnected();
    }
}

//=============================================================================

void TcpHealthCheckProvider::onReconnect(
    void* argument,
    sint8 error)
{
    TcpHealthCheckProvider* provider = providerFromCallback(argument);

    if (provider != nullptr)
    {
        provider->handleReconnect(error);
    }
}

//=============================================================================

void TcpHealthCheckProvider::onDisconnected(void* argument)
{
    TcpHealthCheckProvider* provider = providerFromCallback(argument);

    if (provider != nullptr)
    {
        provider->handleDisconnected();
    }
}

//=============================================================================

void TcpHealthCheckProvider::onDnsFound(
    const char*,
    ipv4_addr_t* address,
    void* argument)
{
    TcpHealthCheckProvider* provider = providerFromCallback(argument);

    if (provider == nullptr ||
        !provider->m_running ||
        !provider->m_dnsPending)
    {
        return;
    }

    if (address == nullptr)
    {
        provider->completeFailure(HealthCheckStatus::DnsFailed);
        return;
    }

    provider->beginConnection(*address);
}

//=============================================================================

void TcpHealthCheckProvider::beginConnection(
    const ipv4_addr_t& address)
{
    if (!m_running)
    {
        return;
    }

    m_dnsPending = false;

    m_tcp.remote_ip[0] = ip4_addr1(&address);
    m_tcp.remote_ip[1] = ip4_addr2(&address);
    m_tcp.remote_ip[2] = ip4_addr3(&address);
    m_tcp.remote_ip[3] = ip4_addr4(&address);

    const sint8 connectResult = espconn_connect(&m_connection);

    if (connectResult != ESPCONN_OK)
    {
        completeFailure(statusFromError(connectResult));
        return;
    }

    m_connectionActive = true;
}

//=============================================================================

void TcpHealthCheckProvider::handleConnected()
{
    if (!m_running)
    {
        return;
    }

    completeSuccess();
}

//=============================================================================

void TcpHealthCheckProvider::handleReconnect(sint8 error)
{
    if (!m_running)
    {
        return;
    }

    m_connectionActive = false;
    completeFailure(statusFromError(error));
}

//=============================================================================

void TcpHealthCheckProvider::handleDisconnected()
{
    if (!m_running)
    {
        return;
    }

    m_connectionActive = false;
    completeFailure(HealthCheckStatus::HostUnreachable);
}

//=============================================================================

void TcpHealthCheckProvider::completeSuccess()
{
    if (!m_running)
    {
        return;
    }

    m_result.success = true;
    m_result.status = HealthCheckStatus::Success;
    m_result.responseTime = millis() - m_startedAt;

    m_running = false;
    m_finished = true;
    m_dnsPending = false;

    Log.verbose(
        "TCP provider: connected to %s:%u in %lu ms",
        m_host,
        static_cast<unsigned>(m_tcp.remote_port),
        static_cast<unsigned long>(m_result.responseTime));

    abortConnection();
}

//=============================================================================

void TcpHealthCheckProvider::completeFailure(HealthCheckStatus status)
{
    if (!m_running)
    {
        return;
    }

    m_result.success = false;
    m_result.status = status;
    m_result.responseTime = millis() - m_startedAt;

    m_running = false;
    m_finished = true;
    m_dnsPending = false;

    Log.verbose(
        "TCP provider: failed to connect to %s:%u, status=%u, time=%lu ms",
        m_host,
        static_cast<unsigned>(m_tcp.remote_port),
        static_cast<uint8_t>(m_result.status),
        static_cast<unsigned long>(m_result.responseTime));

    abortConnection();
}

//=============================================================================

void TcpHealthCheckProvider::abortConnection()
{
    if (!m_connectionActive)
    {
        return;
    }

    m_connectionActive = false;

    espconn_abort(&m_connection);
}

//=============================================================================

HealthCheckStatus TcpHealthCheckProvider::statusFromError(sint8 error) const
{
    if (error == ESPCONN_TIMEOUT)
    {
        return HealthCheckStatus::Timeout;
    }

    if (error == ESPCONN_RTE)
    {
        return HealthCheckStatus::NetworkUnavailable;
    }

    return HealthCheckStatus::HostUnreachable;
}

//=============================================================================

void TcpHealthCheckProvider::reset()
{
    abortConnection();

    m_connection = {};
    m_tcp = {};
    m_resolvedAddress = {};
    m_host[0] = '\0';

    m_result.success = false;
    m_result.status = HealthCheckStatus::Error;
    m_result.responseTime = 0;

    m_startedAt = 0;
    m_timeoutMs = 0;

    m_running = false;
    m_finished = false;
    m_dnsPending = false;
    m_connectionActive = false;
}
