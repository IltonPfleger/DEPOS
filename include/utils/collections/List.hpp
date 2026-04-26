#pragma once

#include <utils/collections/LinkedList.hpp>

namespace DEPOS {

namespace collections {

template <typename T> class List : public LinkedList<T> {
  public:
    using Element = T;

    void insert(T *node) {
        node->m_next = nullptr;
        if (!this->m_head)
            this->m_head = m_tail = node;
        else
            m_tail = (m_tail->m_next = node);
    }

    T *remove() {
        if (!this->m_head) return nullptr;
        T *node      = this->m_head;
        this->m_head = this->m_head->m_next;
        if (!this->m_head) m_tail = nullptr;
        return node;
    }

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

        if (node == m_tail) m_tail = previous;
        return true;
    }

  protected:
    T *m_tail = nullptr;
};

} // namespace collections

} // namespace DEPOS
