#pragma once

#include <Traits.hpp>

namespace DEPOS {

namespace ArchitectureCommon {

template <uintmax_t Duration, void (*Handler)(), size_t Channels> struct TimerTicker {
    void onTick(size_t channel) {
        auto &current = _current[channel];

        if constexpr (Duration > 1)
            if (current-- > 1) [[likely]]
                return;

        Handler();
        current = Duration;
    }

  private:
    uintmax_t _current[Channels] = {Duration};
};

} // namespace ArchitectureCommon

} // namespace DEPOS
