#pragma once

#include <utils/collections/LinkedList.hpp>

namespace DEPOS {

namespace collections {

template <typename T> class LIFO : public LinkedList<T> {
  public:
    using Base = LinkedList<T>;
    using Base::remove;

    LIFO() = default;

    void insert(T *node) {
        node->m_next = this->m_head;
        this->m_head = node;
    }

    T *remove() {
        if (!this->m_head) return nullptr;
        T *node      = this->m_head;
        this->m_head = node->m_next;
        return node;
    }
};

} // namespace collections

} // namespace DEPOS
