#pragma once

#include <Traits.hpp>

#include <BootInformation.hpp>
#include <kernel/Thread.hpp>
#include <shared/memory/Heap.hpp>

namespace DEPOS {

class ApplicationHandler {
  public:
    static void init() {
        auto main = reinterpret_cast<Thread::Return (*)(Thread::Argument)>(__mm.main);
        TraceIn((const void *)main);
        if (main) new Thread(main, 0, Thread::Criterion::NORMAL);
        TraceOut();
    };
};

} // namespace DEPOS
