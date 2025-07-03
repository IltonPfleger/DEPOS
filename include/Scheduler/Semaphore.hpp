#pragma once
#include <CPU.hpp>
#include <Scheduler/Queue.hpp>
#include <Scheduler/Thread.hpp>

struct RawSemaphore {
    Thread::Queue _waiting;
    int _value;

    RawSemaphore(int value) : _value(value) {}

    void p() {
        if (CPU::Atomic::fdec(&_value) < 0) {
            Thread::sleep(&_waiting);
        }
    }

    void v() {
        if (CPU::Atomic::fadd(&_value) <= 0) {
            Thread::wakeup(&_waiting);
        }
    }
};

struct Semaphore {
    Thread::Queue _waiting;
    int _value;

    Semaphore(int value) : _value(value) {}

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
