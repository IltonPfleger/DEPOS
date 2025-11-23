#pragma once

#include <Meta.hpp>

template <typename T = void, typename P = void> struct Element {
    typename Meta::TypeSelector<!Meta::VOID<T>::Result, T>::Result m_value;
    typename Meta::TypeSelector<!Meta::VOID<P>::Result, P>::Result m_priority;
    Element *next = nullptr;

    operator Meta::TypeSelector<!Meta::VOID<T>::Result, T>::Result() const { return m_value; }
};

template <typename T> class LinkedList {
  public:
    using Node = T;
    LinkedList() { m_head = nullptr; };
    bool empty() const { return m_head == nullptr; }
    Node *head() { return m_head; }

  protected:
    Node *m_head;
};

template <typename T> class LIFO : public LinkedList<T> {
  public:
    using Node = T;
    LIFO() = default;

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

template <typename T> class FIFO : public LinkedList<T> {
  public:
    using Node = T;

    FIFO() : LinkedList<T>() { m_tail = nullptr; }

    void insert(Node *n) {
        n->next = nullptr;
        if (!this->m_head) {
            this->m_head = n;
            m_tail = n;
        } else {
            m_tail->next = n;
            m_tail = n;
        }
    }

    Node *remove() {
        if (!this->m_head)
            return nullptr;

        Node *n = this->m_head;
        this->m_head = n->next;

        if (!this->m_head)
            m_tail = nullptr;

        return n;
    }

  private:
    Node *m_tail;
};

template <typename Queue, int Levels> class MLQ {
  public:
    using Node = typename Queue::Node;
    MLQ() = default;

    bool empty() {
        for (auto &l : m_levels) {
            if (!l.empty())
                return false;
        }
        return true;
    }

    void insert(Node *n) { m_levels[n->m_priority].insert(n); }

    Node *remove() {
        for (auto &l : m_levels) {
            if (auto n = l.remove()) {
                return n;
            }
        }
        return nullptr;
    }

  private:
    Queue m_levels[Levels];
};
