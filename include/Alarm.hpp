#pragma once

#include <Spin.hpp>
#include <Thread.hpp>
#include <utility/collections/Node.hpp>
#include <utility/collections/POFO.hpp>

namespace DEPOS {

class Alarm {
    using Node = collections::Node<Thread::Queue, Microsecond>;
    using List = collections::POFO<Node>;

  public:
    Alarm(Microsecond);
    ~Alarm() = default;

    static void delay(Microsecond);
    static void onTick();

  private:
    static bool elapsed(Microsecond);

  private:
    static constinit inline List delays_[Traits<CPU>::Active];

  private:
    Node node_;
};

} // namespace DEPOS
