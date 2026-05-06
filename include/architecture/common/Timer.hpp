#pragma once

#include <Alarm.hpp>
#include <Thread.hpp>
#include <Traits.hpp>
#include <architecture/common/TimerTicker.hpp>

namespace DEPOS {

namespace ArchitectureCommon {

class Timer {
  public:
    static void onTick(size_t channel) {
        Meta::forEach(m_tickers, [channel](auto &ticker) { ticker.onTick(channel); });
    }

  private:
    static constexpr uintmax_t TimerTickFrequency  = Traits<DEPOS::Timer>::TickFrequency;
    static constexpr uintmax_t AlarmTickFrequency  = TimerTickFrequency / Traits<Alarm>::TickFrequency;
    static constexpr uintmax_t ThreadTickFrequency = TimerTickFrequency / Traits<Thread>::TickFrequency;
    static constexpr uintmax_t Active              = Traits<DEPOS::CPU>::Active;

    using ThreadTimerTicker = TimerTicker<ThreadTickFrequency, Thread::onTick, Active>;
    using AlarmTimerTicker  = TimerTicker<AlarmTickFrequency, Alarm::onTick, Active>;

    static inline constinit Meta::Tuple<ThreadTimerTicker, AlarmTimerTicker> m_tickers{};
};

} // namespace ArchitectureCommon

} // namespace DEPOS
