#pragma once
#include <CPU.hpp>
#include <Spin.hpp>
#include <Thread.hpp>

class Semaphore {
    volatile int _value = 1;
    Thread::Queue waiting;
    Spin _lock;

   public:
    void p() {
        Thread::lock();
        _value--;
        if (_value + 1 < 1)
            Thread::sleep(waiting);
        else
            Thread::unlock();
        CPU::Interrupt::enable();
    }

    void v() {
        Thread::lock();
        _value++;
        if (_value - 1 < 0) Thread::wakeup(waiting);
        Thread::unlock();
    }
};
