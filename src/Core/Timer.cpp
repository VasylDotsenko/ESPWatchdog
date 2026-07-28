/*
 * Timer Library
 *
 * ESP Watchdog Project
 *
 * Author: Vasyl Dotsenko
 * Version: 1.0.0
 */

#include "Timer.h"

Timer::Timer()
{
}

Timer::Timer(
    uint32_t interval,
    TimerMode mode)
{
    m_interval = interval;
    m_mode = mode;
}

void Timer::start()
{
    m_started = millis();
    m_running = true;
}

void Timer::start(
    uint32_t interval,
    TimerMode mode)
{
    m_interval = interval;
    m_mode = mode;

    start();
}

void Timer::stop()
{
    m_running = false;
}

void Timer::restart()
{
    start();
}

void Timer::reset()
{
    m_started = millis();
}

bool Timer::expired()
{
    if (!m_running)
    {
        return false;
    }

    const uint32_t now = millis();

    if ((now - m_started) < m_interval)
    {
        return false;
    }

    if (m_mode == TimerMode::Periodic)
    {
        // Синхронізуємо таймер без накопичення дрейфу
        do
        {
            m_started += m_interval;
        }
        while ((now - m_started) >= m_interval);
    }
    else
    {
        m_running = false;
    }

    return true;
}

bool Timer::running() const
{
    return m_running;
}

bool Timer::stopped() const
{
    return !m_running;
}

uint32_t Timer::elapsed() const
{
    if (!m_running)
    {
        return 0;
    }

    return millis() - m_started;
}

uint32_t Timer::remaining() const
{
    if (!m_running)
    {
        return 0;
    }

    const uint32_t elapsedTime = elapsed();

    if (elapsedTime >= m_interval)
    {
        return 0;
    }

    return m_interval - elapsedTime;
}

uint32_t Timer::interval() const
{
    return m_interval;
}

void Timer::setInterval(uint32_t interval)
{
    m_interval = interval;

    if (m_running)
    {
        m_started = millis();
    }
}

TimerMode Timer::mode() const
{
    return m_mode;
}

void Timer::setMode(TimerMode mode)
{
    m_mode = mode;
}