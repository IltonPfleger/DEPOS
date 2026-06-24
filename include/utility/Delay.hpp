#ifndef __QUARK_DELAY__
#define __QUARK_DELAY__

#include <Alarm.hpp>
#include <Timer.hpp>

namespace QUARK {

class Delay {
  public:
    template <typename T> Delay(T delta) { Alarm(Timer::now() + delta); }
};

} // namespace QUARK

#endif
