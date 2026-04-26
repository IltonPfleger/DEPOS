#pragma once

#include <kernel/Thread.hpp>
#include <shared/synchronization/Spin.hpp>
#include <utils/collections/Node.hpp>
#include <utils/collections/POLO.hpp>

namespace DEPOS {

class Alarm {
    using Link = collections::Node<Thread::Queue *, Microsecond>;
    using List = collections::POLO<Link>;

  public:
    static void at(Microsecond);
    static void udelay(Microsecond);
    static void onTick();

  private:
    static bool elapsed(Microsecond);

  private:
    static inline List s_delays[Traits<CPU>::Active];
};

} // namespace DEPOS
