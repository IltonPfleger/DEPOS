#ifndef __DEPOS_DELAY__
#define __DEPOS_DELAY__

#include <Alarm.hpp>
#include <Timer.hpp>

namespace DEPOS {

class Delay {
  public:
    template <typename T> Delay(T delta) { Alarm(Timer::now() + delta); }
};

} // namespace DEPOS

#endif
