#pragma once

#include <Types.hpp>

template <typename T> class LinkedList;
template <typename T> class LIFO;
template <typename T> class FIFO;

template <typename Value = void, typename Priority = void> struct Node {
    friend LinkedList<Node<Value, Priority>>;
    friend LIFO<Node<Value, Priority>>;
    friend FIFO<Node<Value, Priority>>;

    Node *next() const { return m_next; }
    Priority priority() const { return m_priority; }
    Value value() const { return m_value; }

    Node(auto v) : m_value(v) {}
    Node(auto v, auto p) : m_value(v), m_priority(p) {}

  private:
    Meta::IF<Meta::Void<Value>::Result, Meta::Empty, Value>::Result m_value;
    Meta::IF<Meta::Void<Priority>::Result, Meta::Empty, Priority>::Result m_priority;
    Node *m_next = nullptr;
};

template <typename T> class LinkedList {
  public:
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
        node->m_next = this->m_head;
        this->m_head = node;
    }

    T *remove() {
        if (!this->m_head) return nullptr;
        T *node = this->m_head;
        this->m_head = node->m_next;
        return node;
    }

    bool remove(T *node) {
        if (!this->m_head || !node) return false;

        T *previous = nullptr;
        T *current = this->m_head;

        while (current) {
            if (current == node) {
                if (previous) {
                    previous->m_next = current->m_next;
                } else {
                    this->m_head = current->m_next;
                }
                return true;
            }
            previous = current;
            current = current->m_next;
        }
        return false;
    }
};

template <typename T> class FIFO : public LinkedList<T> {
  public:
    FIFO() = default;

    void insert(T *node) {
        node->m_next = nullptr;
        if (!this->m_head) {
            this->m_head = node;
            this->m_tail = node;
        } else {
            this->m_tail->m_next = node;
            this->m_tail = node;
        }
    }

    T *remove() {
        if (!this->m_head) return nullptr;

        T *node = this->m_head;
        this->m_head = this->m_head->m_next;

        return node;
    }

    bool remove(T *node) {
        if (!this->m_head || !node) return false;

        T *previous = nullptr;
        T *current = this->m_head;

        while (current) {
            if (current == node) {
                if (previous) {
                    previous->m_next = current->m_next;
                } else {
                    this->m_head = current->m_next;
                }

                if (current == this->m_tail) {
                    this->m_tail = previous;
                }
                return true;
            }
            previous = current;
            current = current->m_next;
        }
        return false;
    }

  protected:
    T *m_tail = nullptr;
};
