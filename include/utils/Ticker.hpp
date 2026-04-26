#pragma once

template <unsigned long Duration, void (*Handler)(), unsigned int Channels> struct Ticker {
    constexpr Ticker() {
        for (unsigned int i = 0; i < Channels; ++i) {
            m_current[i] = Duration;
        }
    }

    void onTick(unsigned int channel) {
        if (--m_current[channel] <= 0) {
            Handler();
            m_current[channel] = Duration;
        }
    }

  private:
    long m_current[Channels];
};
