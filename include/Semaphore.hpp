#pragma once
#include <CPU.hpp>
#include <IO/Debug.hpp>
#include <Thread.hpp>

class Semaphore {
    int value = 1;
    Thread::Queue waiting;

   public:
    void p() {
        Thread::lock();
        value--;
        if (value + 1 < 1)
            Thread::sleep(waiting);
        else
            Thread::unlock();
        CPU::Interrupt::enable();
    }

    void v() {
        Thread::lock();
        value++;
        if (value - 1 < 0) Thread::wakeup(waiting);
        Thread::unlock();
    }
};
