#pragma once

#include <Memory.hpp>

template <typename T>
struct LIFO {
    struct Node {
        T value;
        Node *next;
    };

    void put(T value) { head = new (Memory::SYSTEM) Node{.value = value, .next = head}; }

    T get() {
        if (!head) return nullptr;
        Node *node = head;
        head       = node->next;
        T value    = node->value;
        delete node;
        return value;
    }

    void remove(T value) {
        Node **current = &head;
        while (*current && (*current)->value != value) {
            current = &(*current)->next;
        }
        if (*current) {
            Node *removed = *current;
            *current      = removed->next;
            delete removed;
        }
    }

    Node *head{nullptr};
};

template <typename T>
struct FIFO {
    struct Node {
        T value;
        Node *next;
    };

    void put(T value) {
        Node *node = new (Memory::SYSTEM) Node{.value = value, .next = nullptr};
        if (!head) {
            head = tail = node;
            return;
        }
        tail->next = node;
        tail       = node;
    }

    T get() {
        if (!head) return nullptr;
        Node *node = head;
        head       = node->next;
        T value    = node->value;
        delete node;
        return value;
    }

    void remove(T value) {
		if(!head) return;
        Node *current  = head;
        Node *previous = nullptr;
        while (current && current->value != value) {
            previous = current;
            current  = current->next;
        }

        if (current == head) {
            head = current->next;
        } else if (current == tail) {
            tail = previous;
        }
        delete current;
    }

    Node *head{nullptr};
    Node *tail{nullptr};
};
