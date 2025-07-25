#pragma once

template <typename T>
struct LinkedList {
    struct Element {
        const T value;
        unsigned long long rank;
        Element* next = nullptr;
    };

    Element* _head = nullptr;
    Element* _tail = nullptr;

    bool empty() const { return _head == nullptr; }

    void push_front(Element* e) {
        e->next = _head;
        _head   = e;
        if (!_tail) _tail = e;
    }

    void push_back(Element* e) {
        e->next = nullptr;
        if (!_head) {
            _head = _tail = e;
        } else {
            _tail->next = e;
            _tail       = e;
        }
    }

    void push_sorted(Element* e) {
        e->next = nullptr;
        if (!_head || e->rank < _head->rank) {
            push_front(e);
        } else {
            Element* current = _head;
            while (current->next && e->rank >= current->next->rank) {
                current = current->next;
            }
            e->next       = current->next;
            current->next = e;
            if (!e->next) _tail = e;
        }
    }

    Element* remove_front() {
        if (!_head) return nullptr;
        Element* e = _head;
        _head      = e->next;
        if (!_head) _tail = nullptr;
        e->next = nullptr;
        return e;
    }

    Element* remove_back() {
        if (!_head) return nullptr;
        if (_head == _tail) {
            Element* e = _head;
            _head = _tail = nullptr;
            return e;
        }
        Element* current = _head;
        while (current->next != _tail) {
            current = current->next;
        }
        Element* e  = _tail;
        _tail       = current;
        _tail->next = nullptr;
        return e;
    }

    void remove(Element* e) {
        if (!_head) return;
        Element* current  = _head;
        Element* previous = nullptr;
        while (current && current != e) {
            previous = current;
            current  = current->next;
        }
        if (!current) return;
        if (current == _head) {
            _head = current->next;
        } else {
            previous->next = current->next;
        }
        if (current == _tail) {
            _tail = previous;
        }
        e->next = nullptr;
        if (!_head) _tail = nullptr;
    }
};

template <typename T>
struct LIFO : private LinkedList<T> {
    using LinkedList<T>::empty;
    using LinkedList<T>::remove;
    using Element = typename LinkedList<T>::Element;

    void insert(Element* value) { LinkedList<T>::push_front(value); }
    Element* next() { return LinkedList<T>::remove_front(); }
};

template <typename T>
struct FIFO : private LinkedList<T> {
    using LinkedList<T>::empty;
    using LinkedList<T>::remove;
    using Element = typename LinkedList<T>::Element;

    void insert(Element* value) { LinkedList<T>::push_back(value); }
    Element* next() { return LinkedList<T>::remove_front(); }
};

template <typename T>
struct POFO : private LinkedList<T> {
    using LinkedList<T>::empty;
    using LinkedList<T>::remove;
    using Element = typename LinkedList<T>::Element;

    void insert(Element* value) { LinkedList<T>::push_sorted(value); }
    Element* next() { return LinkedList<T>::remove_front(); }
};
