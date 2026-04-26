#pragma once

#include <Meta.hpp>

namespace DEPOS {

namespace collections {

template <typename T, bool Tail = false> class LinkedList {
  public:
    LinkedList() = default;

    bool empty() const { return !m_head; }
    T *head() const { return m_head; }

    bool remove(T *node) {
        T *previous = nullptr, *current = this->m_head;
        while (current && current != node) {
            previous = current;
            current  = current->m_next;
        }
        if (!current) return false;

        if (previous)
            previous->m_next = current->m_next;
        else
            this->m_head = current->m_next;

        if constexpr (Tail)
            if (node == m_tail) m_tail = previous;

        return true;
    }

  protected:
    T *m_head = nullptr;
    Meta::IF<Tail, T *, Meta::Empty>::Result m_tail;
};

} // namespace collections

} // namespace DEPOS
