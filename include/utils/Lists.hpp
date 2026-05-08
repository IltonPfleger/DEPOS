#pragma once

#include <Meta.hpp>
#include <types.hpp>
#include <utility/collections/Node.hpp>

namespace DEPOS {

template <typename V = void, typename P = void, typename D = void> using Node = collections::Node<V, P, D>;

template <typename T> class LinkedList {
  public:
    using Element = T;

    bool empty() const { return !m_head; }
    T *head() const { return m_head; }

  protected:
    T *m_head = nullptr;
};

template <typename T> class LIFO : public LinkedList<T> {
  public:
    using Element = T;

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
        return true;
    }
};

template <typename T> class FIFO : public LinkedList<T> {
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

template <typename T> class POLO : public LIFO<T> {
  public:
    using Element = T;
    using LinkedList<T>::m_head;

    void insert(T *node) {
        if (!m_head || node->criterion() < m_head->criterion()) {
            LIFO<T>::insert(node);
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

} // namespace DEPOS
