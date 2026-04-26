#pragma once

#include <utils/collections/LIFO.hpp>

namespace DEPOS {

namespace collections {

template <typename T> class POLO : public collections::LIFO<T> {
  public:
    using Base = collections::LIFO<T>;
    using Base::m_head;

    void insert(T *node) {
        if (!m_head || node->criterion() < m_head->criterion()) {
            Base::insert(node);
        } else {
            T *current = m_head;
            while (current->m_next && current->m_next->criterion() <= node->criterion()) {
                current = current->m_next;
            }

            node->m_next    = current->m_next;
            current->m_next = node;
        }
    }
};

} // namespace collections

} // namespace DEPOS
