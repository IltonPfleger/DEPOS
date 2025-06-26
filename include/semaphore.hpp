#ifndef __semaphore_hpp
#define __semaphore_hpp

#include <thread.hpp>

struct Semaphore {
    Thread::Queue waiting;
    int value;

    static void create(Semaphore* semaphore, int value) { semaphore->value = value; }

    static void p(Semaphore* semaphore) {
        CPU::Interrupt::disable();
        if (CPU::Atomic::fdec(&semaphore->value) < 0) {
            Thread::sleep(&semaphore->waiting);
        }
        CPU::Interrupt::enable();
    }

    static void v(Semaphore* semaphore) {
        CPU::Interrupt::disable();
        if (CPU::Atomic::fadd(&semaphore->value) <= 0) {
            Thread::wakeup(&semaphore->waiting);
        }
        CPU::Interrupt::enable();
    }
};

#endif
