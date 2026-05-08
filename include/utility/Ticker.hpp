#pragma once

#include <Traits.hpp>

namespace DEPOS {

template <uintmax_t Duration, void (*Handler)(), size_t Channels> struct Ticker {
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

} // namespace DEPOS
