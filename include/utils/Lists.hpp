#pragma once

#include <Types.hpp>

template <typename T, typename Priority = void> struct Node {
    const T value;
    Priority priority;
    Node *next = nullptr;
};

template <typename T> struct Node<T, void> {
    const T &value;
    Node *next = nullptr;
};

template <> struct Node<void, void> {
    Node *next = nullptr;
};

template <typename T> class LinkedList {
  public:
    // using DataType = T;
    // using T = Node<T>;
    LinkedList() = default;
    bool empty() const { return m_head == nullptr; }
    T *head() { return m_head; }

  protected:
    T *m_head = nullptr;
};

template <typename T> class LIFO : public LinkedList<T> {
  public:
    // using Base = LinkedList<T>;
    // using T = typename Base::T;
    LIFO() = default;

    void insert(T *node) {
        node->next = this->m_head;
        this->m_head = node;
    }

    T *remove() {
        if (!this->m_head) return nullptr;
        T *node = this->m_head;
        this->m_head = node->next;
        return node;
    }
};

template <typename T> class FIFO : public LinkedList<T> {
  public:
    // using Base = LinkedList<T>;
    // using T = typename Base::T;
    FIFO() = default;

    void insert(T *node) {
        node->next = nullptr;
        if (!this->m_head) {
            this->m_head = node;
            this->m_tail = node;
        } else {
            this->m_tail->next = node;
            this->m_tail = node;
        }
    }

    T *remove() {
        if (!this->m_head) return nullptr;

        T *node = this->m_head;
        this->m_head = this->m_head->next;

        return node;
    }

  protected:
    T *m_tail = nullptr;
};

/* ----------------------------------- O L D
 * -----------------------------------*/

// template <typename T> struct _Node {
//     T value;
//     unsigned long long rank;
//     _Node *next = nullptr;
// };
//
// template <typename T> struct LinkedList {
//     using Node = _Node<T>;
//
//     Node *m_head = nullptr;
//     Node *_tail = nullptr;
//
//     bool empty() const { return m_head == nullptr; }
//
//     void push_front(Node *e) {
//         e->next = m_head;
//         m_head = e;
//         if (!_tail) _tail = e;
//     }
//
//     void push_back(Node *e) {
//         e->next = nullptr;
//         if (!m_head) {
//             m_head = _tail = e;
//         } else {
//             _tail->next = e;
//             _tail = e;
//         }
//     }
//
//     void push_sorted(Node *e) {
//         e->next = nullptr;
//         if (!m_head || e->rank < m_head->rank) {
//             push_front(e);
//         } else {
//             Node *current = m_head;
//             while (current->next && e->rank >= current->next->rank) {
//                 current = current->next;
//             }
//             e->next = current->next;
//             current->next = e;
//             if (!e->next) _tail = e;
//         }
//     }
//
//     Node *remove_front() {
//         if (!m_head) return nullptr;
//         Node *e = m_head;
//         m_head = e->next;
//         if (!m_head) _tail = nullptr;
//         e->next = nullptr;
//         return e;
//     }
//
//     Node *remove_back() {
//         if (!m_head) return nullptr;
//         if (m_head == _tail) {
//             Node *e = m_head;
//             m_head = _tail = nullptr;
//             return e;
//         }
//         Node *current = m_head;
//         while (current->next != _tail) {
//             current = current->next;
//         }
//         Node *e = _tail;
//         _tail = current;
//         _tail->next = nullptr;
//         return e;
//     }
//
//     void remove(Node *e) {
//         if (!m_head) return;
//         Node *current = m_head;
//         Node *previous = nullptr;
//         while (current && current != e) {
//             previous = current;
//             current = current->next;
//         }
//         if (!current) return;
//         if (current == m_head) {
//             m_head = current->next;
//         } else {
//             previous->next = current->next;
//         }
//         if (current == _tail) {
//             _tail = previous;
//         }
//         e->next = nullptr;
//         if (!m_head) _tail = nullptr;
//     }
// };
//
// template <typename T> struct FIFO : private LinkedList<T> {
//     using LinkedList<T>::empty;
//     using LinkedList<T>::remove;
//     using T = typename LinkedList<T>::Node;
//
//     void insert(T *value) { LinkedList<T>::push_back(value); }
//     T *next() { return LinkedList<T>::remove_front(); }
// };
//
// template <typename T> struct POFO : private LinkedList<T> {
//     using LinkedList<T>::empty;
//     using LinkedList<T>::remove;
//     using T = typename LinkedList<T>::Node;
//
//     void insert(T *value) { LinkedList<T>::push_sorted(value); }
//     T *next() { return LinkedList<T>::remove_front(); }
// };
