#pragma once

#include <Thread.hpp>
#include <utility/collections/Node.hpp>
#include <utility/collections/OrderedList.hpp>

namespace QUARK {

class Alarm {

    using Node   = collections::Node<Alarm *, Microsecond, true>;
    using Alarms = collections::OrderedList<Node>;

  public:
    Alarm(Microsecond);
    ~Alarm() = default;

    static void handler();

  private:
    static bool elapsed(Microsecond);

  private:
    static constinit inline Alarms alarms_[Traits<CPU>::Active];

  private:
    Node node_;
    Thread::List thread_;
};

} // namespace QUARK
