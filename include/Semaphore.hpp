#pragma once
#include <CPU.hpp>
#include <Thread.hpp>

struct Semaphore {
    Thread::Queue _waiting;
    int _value;

    Semaphore(int value) { _value = value; }

    void p() {
        CPU::Interrupt::disable();
		if (CPU::Atomic::fdec(&_value) < 0) {
            Thread::sleep(&_waiting);
        }
        CPU::Interrupt::enable();
    }

    void v() {
        CPU::Interrupt::disable();
        if (CPU::Atomic::fadd(&_value) <= 0) {
            Thread::wakeup(&_waiting);
        }
        CPU::Interrupt::enable();
    }
};
