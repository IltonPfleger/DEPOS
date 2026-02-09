#pragma once

template <unsigned long Duration, void (*Handler)(), unsigned int Channels> class Ticker {
  public:
    void tick(unsigned int channel) {
        if (channel < Channels && --m_current[channel] == 0) {
            Handler();
            reset(channel);
        }
    }

    void reset(unsigned int channel) { m_current[channel] = Duration; }

  public:
    static constexpr unsigned int k_channels = Channels;
    static constexpr unsigned int k_duration = Duration;

  private:
	unsigned long m_current[Channels + 1] = {Duration};
};
