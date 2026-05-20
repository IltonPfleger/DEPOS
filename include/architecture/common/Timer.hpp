#pragma once

#include <Alarm.hpp>
#include <Thread.hpp>
#include <Traits.hpp>
#include <utility/Ticker.hpp>

namespace DEPOS {

namespace ArchitectureCommon {

class Timer {
  public:
    static void onTick(size_t channel) {
        Meta::forEach(m_tickers, [channel](auto &ticker) { ticker.onTick(channel); });
    }

  private:
    static constexpr uintmax_t TimerFrequency  = Traits<DEPOS::Timer>::Frequency;
    static constexpr uintmax_t AlarmFrequency  = TimerFrequency / Traits<Alarm>::Frequency;
    static constexpr uintmax_t ThreadFrequency = TimerFrequency / Traits<Thread>::Frequency;
    static constexpr uintmax_t Active          = Traits<DEPOS::CPU>::Active;

    using ThreadTicker = Ticker<ThreadFrequency, Thread::onTick, Active>;
    using AlarmTicker  = Ticker<AlarmFrequency, Alarm::onTick, Active>;

    static inline constinit Meta::Tuple<ThreadTicker, AlarmTicker> m_tickers{};
};

} // namespace ArchitectureCommon

} // namespace DEPOS
