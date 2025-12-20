#pragma once

#include <Task.hpp>
#include <Thread.hpp>
#include <Traits.hpp>
#include <memory/Heap.hpp>
#include <memory/MemoryMap.hpp>

class Application {
  public:
    static void init() {
        TraceIn();
        auto main = reinterpret_cast<int (*)(void *)>(__mm.main);
        Console::println("%d\n", reinterpret_cast<void *>(main));
        if (main)
            new (Heap::SYSTEM) Thread(main, 0, Thread::Criterion::NORMAL);
        TraceOut();
    };
};
