#pragma once

#include <Memory.hpp>

template <typename T>
struct Stack {
    struct Node {
        T value;
        Node *next;
    };

    void put(T value) {
        Node *node = new (Memory::SYSTEM) Node{.value = value, .next = head};
        head       = node;
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
