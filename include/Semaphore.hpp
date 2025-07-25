#pragma once
#include <CPU.hpp>
#include <Thread.hpp>

class Semaphore {
    Thread::Queue waiting;
    int value;

   public:
    Semaphore(int value) : value(value) {}

    void p() {
        if (CPU::Atomic::fdec(&value) < 0) Thread::sleep(&waiting);
    }

    void v() {
        if (CPU::Atomic::fadd(&value) <= 0) Thread::wakeup(&waiting);
    }
};
