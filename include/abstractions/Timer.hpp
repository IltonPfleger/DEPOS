#pragma once

#include <Macros.hpp>
#include <Thread.hpp>
#include <Traits.hpp>
#include <utils/Ticker.hpp>

#include __TIMER_HEADER

using Ticker1 = Ticker<Traits<Timer>::Frequency / Traits<Thread>::RescheduleFrequency, Thread::reschedule, Traits<CPU>::Active>;

class Timer : public __ARCH::Timer<Ticker1> {};
