#pragma once
#include <CPU.hpp>
#include <Thread.hpp>

class Semaphore {
    Thread::Queue waiting;
    volatile int value;

   public:
    Semaphore(int value) : value(value) {}

    void p() {
        CPU::Interrupt::disable();
        if (CPU::Atomic::fdec(&value) < 0) Thread::sleep(&waiting);
        CPU::Interrupt::enable();
    }

    void v() {
        CPU::Interrupt::disable();
        if (CPU::Atomic::fadd(&value) <= 0) Thread::wakeup(&waiting);
        CPU::Interrupt::enable();
    }
};
