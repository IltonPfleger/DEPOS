#pragma once

#include <Types.hpp>

template <typename T> struct Element {
    const T &data;
    Element *next;
};

template <> struct Element<void> {
    Element *next;
};

template <typename T> class LinkedListBase {
  public:
    using DataType = T;
    using Node = Element<T>;
    bool empty() const { return m_head == nullptr; }
    Node *head() { return m_head; }

  protected:
    Node *m_head = nullptr;
};

template <typename T> class LIFO : public LinkedListBase<T> {
  public:
    using Base = LinkedListBase<T>;
    using Node = typename Base::Node;

    void insert(Node *node) {
        node->next = this->m_head;
        this->m_head = node;
    }

    Node *remove() {
        if (!this->m_head)
            return nullptr;
        Node *node = this->m_head;
        this->m_head = node->next;
        return node;
    }
};

/* ----------------------------------- O L D
 * -----------------------------------*/

template <typename T> struct _Element {
    T value;
    unsigned long long rank;
    _Element *next = nullptr;
};

template <typename T> struct LinkedList {
    using Element = _Element<T>;

    Element *m_head = nullptr;
    Element *_tail = nullptr;

    bool empty() const { return m_head == nullptr; }

    void push_front(Element *e) {
        e->next = m_head;
        m_head = e;
        if (!_tail)
            _tail = e;
    }

    void push_back(Element *e) {
        e->next = nullptr;
        if (!m_head) {
            m_head = _tail = e;
        } else {
            _tail->next = e;
            _tail = e;
        }
    }

    void push_sorted(Element *e) {
        e->next = nullptr;
        if (!m_head || e->rank < m_head->rank) {
            push_front(e);
        } else {
            Element *current = m_head;
            while (current->next && e->rank >= current->next->rank) {
                current = current->next;
            }
            e->next = current->next;
            current->next = e;
            if (!e->next)
                _tail = e;
        }
    }

    Element *remove_front() {
        if (!m_head)
            return nullptr;
        Element *e = m_head;
        m_head = e->next;
        if (!m_head)
            _tail = nullptr;
        e->next = nullptr;
        return e;
    }

    Element *remove_back() {
        if (!m_head)
            return nullptr;
        if (m_head == _tail) {
            Element *e = m_head;
            m_head = _tail = nullptr;
            return e;
        }
        Element *current = m_head;
        while (current->next != _tail) {
            current = current->next;
        }
        Element *e = _tail;
        _tail = current;
        _tail->next = nullptr;
        return e;
    }

    void remove(Element *e) {
        if (!m_head)
            return;
        Element *current = m_head;
        Element *previous = nullptr;
        while (current && current != e) {
            previous = current;
            current = current->next;
        }
        if (!current)
            return;
        if (current == m_head) {
            m_head = current->next;
        } else {
            previous->next = current->next;
        }
        if (current == _tail) {
            _tail = previous;
        }
        e->next = nullptr;
        if (!m_head)
            _tail = nullptr;
    }
};

template <typename T> struct FIFO : private LinkedList<T> {
    using LinkedList<T>::empty;
    using LinkedList<T>::remove;
    using Node = typename LinkedList<T>::Element;

    void insert(Node *value) { LinkedList<T>::push_back(value); }
    Node *next() { return LinkedList<T>::remove_front(); }
};

template <typename T> struct POFO : private LinkedList<T> {
    using LinkedList<T>::empty;
    using LinkedList<T>::remove;
    using Node = typename LinkedList<T>::Element;

    void insert(Node *value) { LinkedList<T>::push_sorted(value); }
    Node *next() { return LinkedList<T>::remove_front(); }
};
