#pragma once

#include <architecture/CPU.hpp>

namespace DEPOS {

namespace collections {

template <typename T> class Atomic {
  public:
    Atomic()
        : m_head(nullptr) {}

    void insert(T *node) {
        T *head;
        do {
            head         = m_head;
            node->next() = head;
        } while (!CPU::Atomic::cas(m_head, head, node));
    }

    T *remove() {
        T *old;
        T *next;
        do {
            old = m_head;
            if (!old) return nullptr;
            next = old->next();
        } while (!CPU::Atomic::cas(m_head, old, next));
        return old;
    }

  protected:
    T *volatile m_head;
};

} // namespace collections

} // namespace DEPOS
