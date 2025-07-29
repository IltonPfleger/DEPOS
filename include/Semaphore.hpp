#pragma once
#include <CPU.hpp>
#include <Thread.hpp>

class Semaphore {
    Thread::Queue waiting;
    volatile int value;

   public:
    Semaphore(int value) : value(value) {}

    void p() {
        Thread::spin.lock();
        if (CPU::Atomic::fdec(&value) < 0) {
            Thread::sleep(&waiting);
        } else {
            Thread::spin.unlock();
        }
    }

    void v() {
        Thread::spin.lock();
        if (CPU::Atomic::fadd(&value) <= 0) Thread::wakeup(&waiting);
        Thread::spin.unlock();
    }
};
