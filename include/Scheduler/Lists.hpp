#pragma once

#include <Memory.hpp>
#include <Meta.hpp>

// TODO: If we have a doubly linked list, remove from tail is faster

template <typename T>
struct LinkedList {
    struct Element {
        T value;
        Element *next;
    };

    bool empty() const { return head == nullptr; }

    T remove_front() {
        if (!head) return T{};
        Element *e = head;
        head       = e->next;
        T value    = e->value;
        if (!head) tail = nullptr;
        delete e;
        return value;
    }

    T remove_back() {
        if (!head) return T{};
        if (head == tail) {
            T value = head->value;
            delete head;
            head = tail = nullptr;
            return value;
        }

        Element *current = head;
        while (current->next != tail) {
            current = current->next;
        }

        T value = tail->value;
        delete tail;
        tail       = current;
        tail->next = nullptr;
        return value;
    }

    void push_front(const T &value) {
        head = new (Memory::SYSTEM) Element{value, head};
        if (!tail) tail = head;
    }

    void push_back(const T &value) {
        Element *e = new (Memory::SYSTEM) Element{value, nullptr};
        if (!head) {
            head = tail = e;
        } else {
            tail->next = e;
            tail       = e;
        }
    }

    void push_sorted(const T &value)
        requires requires(const T &v) { v(); } || requires(const T &v) { (*v)(); }
    {
        Element *e = new (Memory::SYSTEM) Element{value, nullptr};

        if (!head || rank(value) > rank(head->value)) {
            e->next = head;
            head    = e;
            if (!tail) tail = e;
        } else {
            Element *current = head;
            while (current->next && rank(value) > rank(current->next->value)) {
                current = current->next;
            }
            e->next       = current->next;
            current->next = e;
            if (!e->next) tail = e;
        }
    }

    constexpr auto rank(const T &value)
        requires requires(const T &v) { v(); } || requires(const T &v) { (*v)(); }
    {
        if constexpr (Meta::IS_POINTER<T>::Result) {
            return (*value)();
        } else {
            return value();
            ;
        }
    }

    void remove(const T &value) {
        Element *current  = head;
        Element *previous = nullptr;

        while (current && current->value != value) {
            previous = current;
            current  = current->next;
        }

        if (!current) return;

        if (current == head) {
            head = current->next;
        } else {
            previous->next = current->next;
        }

        if (current == tail) {
            tail = previous;
        }

        delete current;

        if (!head) tail = nullptr;
    }

    Element *head = nullptr;
    Element *tail = nullptr;
};

template <typename T>
struct LIFO : private LinkedList<T> {
    using LinkedList<T>::empty;
    using LinkedList<T>::remove;
    void insert(const T &value) { LinkedList<T>::push_front(value); }
    T next() { return LinkedList<T>::remove_front(); }
};

template <typename T>
struct FIFO : private LinkedList<T> {
    using LinkedList<T>::empty;
    using LinkedList<T>::remove;
    void insert(const T &value) { LinkedList<T>::push_back(value); }
    T next() { return LinkedList<T>::remove_front(); }
};

template <typename T>
struct POFO : private LinkedList<T> {
    using LinkedList<T>::empty;
    using LinkedList<T>::remove;
    void insert(const T &value) { LinkedList<T>::push_sorted(value); }
    T next() { return LinkedList<T>::remove_front(); }
};
