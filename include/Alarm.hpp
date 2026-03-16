#pragma once

#include <Semaphore.hpp>
#include <Spin.hpp>
#include <Thread.hpp>

namespace DEPOS {

class Alarm {
    using Link = Node<Thread::Queue *, Microsecond>;
    using List = DEPOS::POLO<Link>;

  public:
    static void at(Microsecond);
    static void udelay(Microsecond);
    static void handler();

  private:
    static bool elapsed(Microsecond);

  private:
    static inline List s_delays[Traits<CPU>::Active];
};

} // namespace DEPOS
