#pragma once

#include <Alarm.hpp>
#include <Thread.hpp>
#include <Traits.hpp>
#include <utility/Ticker.hpp>

namespace DEPOS {

namespace ArchitectureCommon {

class Timer {
    static constexpr uintmax_t TimerFrequency  = Traits<DEPOS::Timer>::Frequency;
    static constexpr uintmax_t AlarmFrequency  = TimerFrequency / Traits<Alarm>::Frequency;
    static constexpr uintmax_t ThreadFrequency = TimerFrequency / Traits<Thread>::Frequency;
    static constexpr uintmax_t Active          = Traits<DEPOS::CPU>::Active;
    using ThreadTicker                         = Ticker<ThreadFrequency, Thread::onTick, Active>;
    using AlarmTicker                          = Ticker<AlarmFrequency, Alarm::onTick, Active>;

  protected:
    static void onTick(size_t channel) {
        Meta::forEach(tickers_, [channel]<typename T>(T &ticker) {
            if constexpr (!Meta::Same<T, Meta::Empty>::Result) {
                ticker.onTick(channel);
            }
        });
    }

  private:
    static inline constinit Meta::Tuple<ThreadTicker, AlarmTicker> tickers_{};
};

} // namespace ArchitectureCommon

} // namespace DEPOS
