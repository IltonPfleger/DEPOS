#pragma once

#include <Traits.hpp>
#include <utils/Ticker.hpp>

namespace ArchitectureCommon {

template <typename... Tickers> class TimerTemplate : public Tickers... {
  public:
    static void handler(unsigned int channel) { (instance()->template update<Tickers>(channel), ...); }

  private:
    TimerTemplate() = default;
    template <typename T> void update(unsigned int channel) { static_cast<T *>(this)->tick(channel); }
    static TimerTemplate *instance() {
        static TimerTemplate $;
        return &$;
    }
};

} // namespace ArchitectureCommon
