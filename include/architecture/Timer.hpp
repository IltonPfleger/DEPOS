#pragma once


#include <Alarm.hpp>
#include <Macros.hpp>
#include <Thread.hpp>
#include <Traits.hpp>
#include <utils/Ticker.hpp>

#include __TIMER_HEADER

namespace DEPOS {

using Ticker1 = Ticker<Traits<Timer>::Frequency / Traits<Thread>::RescheduleFrequency,
                       Thread::reschedule,
                       Traits<CPU>::Active>;

using Ticker2 = Ticker<Traits<Timer>::Frequency / Traits<Alarm>::Frequency, Alarm::handler, 1>;

class Timer : public __ARCH::Timer<Ticker1, Ticker2> {};

} // namespace DEPOS
