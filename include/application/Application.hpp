#pragma once

// #include <Task.hpp>
#include <Thread.hpp>
#include <Traits.hpp>
#include <memory/Heap.hpp>
#include <memory/MemoryMap.hpp>

class Application {
  public:
    static void init() {
        auto main = reinterpret_cast<Thread::Return (*)(Thread::Argument)>(__mm.main);
        TraceIn(main);
        if (main) new (Heap::SYSTEM) Thread(main, 0, Thread::Criterion::NORMAL);
        TraceOut();
    };
};
