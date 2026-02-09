#pragma once

#include <Meta.hpp>
#include <Types.hpp>

template <typename V, typename P> using ValueType = typename Meta::IF<Meta::Void<V>::Result, Meta::Empty, V>::Result;

template <typename V, typename P> using PriorityType = typename Meta::IF<Meta::Void<P>::Result, Meta::Empty, P>::Result;

template <typename Value = void, typename Priority = void> struct Node {
    template <typename T> friend class LinkedList;
    template <typename T> friend class LIFO;
    template <typename T> friend class FIFO;

    Node *next() const { return m_next; }
    auto priority() const { return m_priority; }
    auto value() const { return m_value; }

    Node(auto v) : m_value(v) {}
    Node(auto v, auto p) : m_value(v), m_priority(p) {}

  private:
    ValueType<Value, Priority> m_value;
    PriorityType<Value, Priority> m_priority;
    Node *m_next = nullptr;
};

template <typename T>
concept List = requires(T list, typename T::Element *node) {
    list.insert(node);
    { list.insert(node) } -> Meta::SameAs<void>;
    { list.remove() } -> Meta::SameAs<typename T::Element *>;
};

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
        T *node = this->m_head;
        this->m_head = node->m_next;
        return node;
    }

    bool remove(T *node) {
        T *previous = nullptr, *current = this->m_head;
        while (current && current != node) {
            previous = current;
            current = current->m_next;
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
        T *node = this->m_head;
        this->m_head = this->m_head->m_next;
        if (!this->m_head) m_tail = nullptr;
        return node;
    }

    bool remove(T *node) {
        T *previous = nullptr, *current = this->m_head;
        while (current && current != node) {
            previous = current;
            current = current->m_next;
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
