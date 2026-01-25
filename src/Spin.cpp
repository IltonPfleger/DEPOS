#include <machine/Machine.hpp>

void Spin::acquire() {
    while (__atomic_test_and_set(&locked, __ATOMIC_SEQ_CST))
        ;
}

void Spin::release() { __atomic_clear(&locked, __ATOMIC_SEQ_CST); }

void Spin::lock() {
    auto i = CPU::Interruptions::off();
    acquire();
    interrupts = i;
}

void Spin::unlock() {
    auto i = interrupts;
    release();
    if (i) CPU::Interruptions::on();
}
