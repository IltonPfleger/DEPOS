#pragma once

#include <Task.hpp>
#include <Thread.hpp>
#include <Traits.hpp>
#include <memory/Heap.hpp>
#include <memory/MemoryMap.hpp>

class Application {
  public:
    static void init() {
        // if constexpr (Traits::System::MULTITASK) {
        //    uintptr_t CodeStart = reinterpret_cast<uintptr_t>(__mm.app.code.start);
        //    uintptr_t CodeEnd   = reinterpret_cast<uintptr_t>(__mm.app.code.end);
        //    TraceIn(CodeStart, CodeEnd);

        //    Task *task   = new (Heap::SYSTEM) Task();
        //    Segment code = Segment(__mm.app.code.start, CodeEnd - CodeStart);
        //    task->attach(code);

        //    new (Heap::SYSTEM)
        //        Thread(reinterpret_cast<int (*)(void *)>(__mm.app.entry), 0, Thread::Criterion::NORMAL, task);
        //    TraceOut();

        //} else {
        new (Heap::SYSTEM) Thread(reinterpret_cast<int (*)(void *)>(__mm.main), 0, Thread::Criterion::NORMAL);
        //}
    };
};
