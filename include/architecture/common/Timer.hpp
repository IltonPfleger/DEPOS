#pragma once

#include <Traits.hpp>
#include <utils/Ticker.hpp>

namespace ArchitectureCommon {

template <typename... Tickers> class TimerTemplate : public Tickers... {

  public:
    static TimerTemplate &instance() { return _; }

    static void handler(unsigned int channel) { (instance().template update<Tickers>(channel), ...); }

  private:
    TimerTemplate() = default;

    template <typename T> void update(unsigned int channel) {
        T &ticker = static_cast<T &>(*this);
        ticker.tick(channel);
    }

  private:
    static inline TimerTemplate _;
};

} // namespace ArchitectureCommon
