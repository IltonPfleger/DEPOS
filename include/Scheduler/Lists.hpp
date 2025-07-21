#pragma once

template <typename T>
struct LinkedList {
    struct Element {
        const T value;
        unsigned long long rank;
		Element* next;
    };

    Element* head = nullptr;
    Element* tail = nullptr;

    bool empty() const { return head == nullptr; }

    void push_front(Element* e) {
        e->next = head;
        head    = e;
        if (!tail) tail = e;
    }

    void push_back(Element* e) {
        e->next = nullptr;
        if (!head) {
            head = tail = e;
        } else {
            tail->next = e;
            tail       = e;
        }
    }

    void push_sorted(Element* e) {
        e->next = nullptr;
        if (!head || e->rank < head->rank) {
            push_front(e);
        } else {
            Element* current = head;
            while (current->next && e->rank >= current->next->rank) {
                current = current->next;
            }
            e->next       = current->next;
            current->next = e;
            if (!e->next) tail = e;
        }
    }

    Element* remove_front() {
        if (!head) return nullptr;
        Element* e = head;
        head       = e->next;
        if (!head) tail = nullptr;
        e->next = nullptr;
        return e;
    }

    Element* remove_back() {
        if (!head) return nullptr;
        if (head == tail) {
            Element* e = head;
            head = tail = nullptr;
            return e;
        }
        Element* current = head;
        while (current->next != tail) {
            current = current->next;
        }
        Element* e = tail;
        tail       = current;
        tail->next = nullptr;
        return e;
    }

    void remove(Element* e) {
        if (!head) return;
        Element* current  = head;
        Element* previous = nullptr;
        while (current && current != e) {
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
        e->next = nullptr;
        if (!head) tail = nullptr;
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
