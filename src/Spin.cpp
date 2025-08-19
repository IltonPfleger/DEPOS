#include <CPU.hpp>
#include <Spin.hpp>

void Spin::acquire() { while (__atomic_test_and_set(&locked, __ATOMIC_SEQ_CST)); }

void Spin::release() { __atomic_clear(&locked, __ATOMIC_SEQ_CST); }

void Spin::lock() {
    volatile auto i = CPU::Interrupt::off();
    acquire();
    interrupts = i;
}

void Spin::unlock() {
    volatile auto i = interrupts;
    release();
    if (i) CPU::Interrupt::on();
};
