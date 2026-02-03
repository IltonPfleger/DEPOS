#include <machine/Machine.hpp>

void Spin::acquire() {
    while (__atomic_test_and_set(&m_locked, __ATOMIC_SEQ_CST))
        ;
}

void Spin::release() { __atomic_clear(&m_locked, __ATOMIC_SEQ_CST); }
