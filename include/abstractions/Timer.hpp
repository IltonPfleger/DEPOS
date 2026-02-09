#pragma once

#include <Thread.hpp>
#include <Traits.hpp>
#include <machine/Machine.hpp>
#include <utils/Ticker.hpp>

using ThreadTicker =
    Ticker<Traits<Timer>::Frequency / Traits<Thread>::RescheduleFrequency, Thread::reschedule, Traits<CPU>::Active>;

class Timer : public Machine::Timer<ThreadTicker> {};
