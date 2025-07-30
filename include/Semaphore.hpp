#pragma once
#include <CPU.hpp>
#include <IO/Debug.hpp>
#include <Thread.hpp>

class Semaphore {
    volatile int value = 1;
    Thread::Queue waiting;
    Spin spin;

   public:
    void p() {
        Thread::lock();
        if (CPU::Atomic::fdec(value) < 1)
            Thread::sleep(waiting);
        else
            Thread::unlock();
        CPU::Interrupt::enable();
    }

    void v() {
        Thread::lock();
        if (CPU::Atomic::finc(value) < 0) Thread::wakeup(waiting);
        Thread::unlock();
    }
};
