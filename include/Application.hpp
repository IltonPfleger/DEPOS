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
        // auto main = reinterpret_cast<int (*)(void *)>(__mm.main);
        //  if constexpr (Traits<System>::MULTITASK) {
        //   uintptr_t TextStart = __mm.text.start;
        //   uintptr_t TextEnd = __mm.text.end;
        //   uintptr_t TextSize = TextEnd - TextStart;

        // uintptr_t DataStart = __mm.data.start;
        // uintptr_t DataEnd = __mm.data.end;
        // uintptr_t DataSize = DataEnd - DataStart;

        // uintptr_t BssStart = __mm.bss.start;
        // uintptr_t BssEnd = __mm.bss.end;
        // uintptr_t BssSize = BssEnd - BssStart;

        // TraceIn(TextStart, TextSize, DataStart, DataSize, BssStart,
        //         BssSize);

        // Task *task = new (Heap::SYSTEM) Task();
        ////    Segment text = Segment(__mm.app.text.start, TextEnd -
        ////    TextStart); task->attach(text);

        ////    new (Heap::SYSTEM)
        ////        Thread(reinterpret_cast<int (*)(void
        ///*)>(__mm.app.entry), /        0, Thread::Criterion::NORMAL,
        /// task); /    TraceOut();
        // new (Heap::SYSTEM) Thread(main, 0, Thread::Criterion::NORMAL,
        // task);

        //} else {
        // if (main)
        //    new (Heap::SYSTEM) Thread(main, 0, Thread::Criterion::NORMAL);
        // }
        TraceOut();
    };
};
