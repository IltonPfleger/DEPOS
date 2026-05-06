#pragma once

#include <BootInformation.hpp>
#include <Thread.hpp>
#include <Traits.hpp>
#include <memory/Heap.hpp>

namespace DEPOS {

class Application {
  public:
    static void init() {
        auto main = reinterpret_cast<Thread::Return (*)(Thread::Argument)>(__mm.main);
        if (main) new Thread(main, 0, Thread::Criterion::NORMAL);
    };
};

} // namespace DEPOS
