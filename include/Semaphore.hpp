#pragma once
#include <CPU.hpp>
#include <Thread.hpp>

class Semaphore {
    volatile int value = 1;
    Thread::Queue waiting;

   public:
    void p() {
        CPU::Interrupt::disable();
        if (CPU::Atomic::fdec(value) < 1)
            Thread::sleep(waiting);
        else
            CPU::Interrupt::enable();
    }

    void v() {
        CPU::Interrupt::disable();
        if (CPU::Atomic::finc(value) < 0) Thread::wakeup(waiting);
        CPU::Interrupt::enable();
    }
};
