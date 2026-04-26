#pragma once

#include <utils/collections/LinkedList.hpp>

namespace DEPOS {

namespace collections {

template <typename T> class FIFO : public LinkedList<T, true> {
  public:
    using Base = LinkedList<T, true>;
    using Base::remove;

    FIFO() = default;

    void insert(T *node) {
        node->m_next = nullptr;
        if (!this->m_head)
            this->m_head = this->m_tail = node;
        else
            this->m_tail = (this->m_tail->m_next = node);
    }

    T *remove() {
        if (!this->m_head) return nullptr;
        T *node      = this->m_head;
        this->m_head = this->m_head->m_next;
        if (!this->m_head) this->m_tail = nullptr;
        return node;
    }
};

} // namespace collections

} // namespace DEPOS
