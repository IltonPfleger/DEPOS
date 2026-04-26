#include <architecture/CPU.hpp>
#include <shared/synchronization/Spin.hpp>

using namespace DEPOS;

void Spin::acquire() {
    while (CPU::Atomic::tsl(m_locked))
        ;
}

void Spin::release() { m_locked = 0; }
