#pragma once

#include <architecture/CPU.hpp>

namespace DEPOS {

template <typename T> class MPSC {

  public:
    MPSC() { m_head = m_tail = &m_stub; }

    // Multiple-Producers
    void insert(T *node) {
        node->next = nullptr;
        T *old;
        do {
            old = CPU::Atomic::load(&m_tail);
        } while (!CPU::Atomic::cas(&m_tail, old, node));
        old->next = node;
    }

    // Single-Consumer
    T *remove() {
        T *head = m_head;
        T *next = head->next;

        if (head == &m_stub) {
            if (next == nullptr) {
                return nullptr;
            }
            m_head = next;
            head   = next;
            next   = next->next;
        }

        if (next != nullptr) {
            m_head = next;
            return head;
        }

        // Just-in-Case
        if (head != CPU::Atomic::load(&m_tail)) {
            while ((next = head->next) == nullptr)
                ;
            m_head = next;
            return head;
        }

        return nullptr;
    }

  protected:
    T m_stub;
    T *m_head = nullptr;
    T *m_tail = nullptr;
};

} // namespace DEPOS
