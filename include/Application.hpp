#pragma once

#include <Task.hpp>
#include <Thread.hpp>
#include <Traits.hpp>
#include <memory/Heap.hpp>
#include <memory/MemoryMap.hpp>

class Application {
   public:
    static void init() {
        if constexpr (Traits::System::MULTITASK) {
        } else {
            new (Heap::SYSTEM) Thread(reinterpret_cast<int (*)(void*)>(__mm.app.entry), 0, Thread::Criterion::NORMAL);
        }
    };
};
