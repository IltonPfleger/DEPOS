#pragma once

template <unsigned long Duration, void (*Handler)(), unsigned int Channels> class Ticker {
  public:
    Ticker() {
        for (unsigned int i = 0; i < Channels; ++i) {
            m_current[i] = Duration;
        }
    }

    void tick(unsigned int channel) {
        if (channel >= Channels) return;

        if (--m_current[channel] <= 0) {
            Handler();
            m_current[channel] = Duration;
        }
    }

  public:
    static constexpr unsigned int k_channels = Channels;
    static constexpr unsigned int k_duration = Duration;

  private:
    long m_current[Channels];
};
