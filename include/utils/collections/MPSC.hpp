#pragma once

#include <architecture/CPU.hpp>

namespace DEPOS {

namespace collections {

template <typename T> class MPSC {
  public:
    MPSC() {
        m_stub.m_next = nullptr;
        m_head        = &m_stub;
        m_tail        = &m_stub;
    }

    void insert(T *node) {
        node->m_next = nullptr;
        T *old       = CPU::Atomic::exchange(m_head, node);
        CPU::Atomic::store(old->m_next, node);
    }

    T *remove() {
        T *tail = m_tail;
        T *next = CPU::Atomic::load(tail->m_next);

        if (tail == &m_stub) {
            if (!next) return nullptr;

            m_tail = next;
            tail   = next;
            next   = CPU::Atomic::load(tail->m_next);
        }

        if (next) {
            m_tail = next;
            return tail;
        }

        if (tail != CPU::Atomic::load(m_head)) return nullptr;

        insert(&m_stub);

        next = CPU::Atomic::load(tail->m_next);
        if (next) {
            m_tail = next;
            return tail;
        }

        return nullptr;
    }

  protected:
    T m_stub;
    T *m_head;
    T *m_tail;
};

} // namespace collections

} // namespace DEPOS
