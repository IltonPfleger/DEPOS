#pragma once

#include <Spin.hpp>
#include <Thread.hpp>
#include <utility/collections/Node.hpp>
#include <utility/collections/POLO.hpp>

namespace DEPOS {

class Alarm {
    using Link = collections::Node<Thread::Queue, Microsecond>;
    using List = collections::POLO<Link>;

  public:
    static void at(Microsecond);
    static void udelay(Microsecond);
    static void onTick();

  private:
    static bool elapsed(Microsecond);

  private:
    static constinit inline List delays_[Traits<CPU>::Active];
};

} // namespace DEPOS
