/*
 * Timer Library
 *
 * ESP Watchdog Project
 *
 * Author: Vasyl Dotsenko
 * Version: 1.0.0
 */

#pragma once

#include <Arduino.h>

enum class TimerMode
{
    OneShot,
    Periodic
};

class Timer
{
public:

    Timer();

    explicit Timer(
        uint32_t interval,
        TimerMode mode = TimerMode::Periodic);

    void start();

    void start(
        uint32_t interval,
        TimerMode mode = TimerMode::Periodic);

    void stop();

    void restart();

    void reset();

    bool expired();

    bool running() const;

    bool stopped() const;

    uint32_t elapsed() const;

    uint32_t remaining() const;

    uint32_t interval() const;

    void setInterval(uint32_t interval);

    TimerMode mode() const;

    void setMode(TimerMode mode);

private:

    uint32_t m_interval = 0;

    uint32_t m_started = 0;

    TimerMode m_mode = TimerMode::Periodic;

    bool m_running = false;
};