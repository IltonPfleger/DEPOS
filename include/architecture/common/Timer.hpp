#pragma once

#include <Meta.hpp>
#include <Traits.hpp>
#include <kernel/Alarm.hpp>
#include <kernel/Thread.hpp>
#include <utils/Ticker.hpp>

namespace DEPOS {

namespace ArchitectureCommon {

class Timer {
    static constexpr uintmax_t Active              = Traits<DEPOS::CPU>::Active;
    static constexpr uintmax_t TimerTickFrequency  = Traits<DEPOS::Timer>::TickFrequency;
    static constexpr uintmax_t AlarmTickFrequency  = TimerTickFrequency / Traits<Alarm>::TickFrequency;
    static constexpr uintmax_t ThreadTickFrequency = TimerTickFrequency / Traits<Thread>::TickFrequency;
    using ThreadTicker                             = Ticker<ThreadTickFrequency, Thread::onTick, Active>;
    using AlarmTicker                              = Ticker<AlarmTickFrequency, Alarm::onTick, Active>;

  public:
    static void onTick(unsigned int channel) {
        meta::forEach(m_tickers, [channel](auto &ticker) { ticker.onTick(channel); });
    }

  private:
    static inline constinit meta::Tuple<ThreadTicker, AlarmTicker> m_tickers{};
};

} // namespace ArchitectureCommon

} // namespace DEPOS
