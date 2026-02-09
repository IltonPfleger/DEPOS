#pragma once

#include <Thread.hpp>
#include <Traits.hpp>
#include <machine/Machine.hpp>
#include <utils/Ticker.hpp>

using SchedulerTicker =
    Ticker<(Traits<Timer>::Frequency * Traits<Thread>::Quantum) / 1'000'000, Thread::reschedule, Traits<CPUS>::ACTIVE>;

class Timer : public TimerTemplate<SchedulerTicker> {};
