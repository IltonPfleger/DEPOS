#ifndef QUEUE_H
#define QUEUE_H

#include <memory.hpp>

template <typename T>
struct Queue {
    struct Node {
        T value;
        struct Node* next;
    };

    struct Node* head;
    struct Node* tail;

    Queue() : head(nullptr), tail(nullptr) {}

    inline bool empty() const { return head == nullptr; }

    void push(T value) {
        struct Node* node = (struct Node*)Memory::malloc(sizeof(Node));
        node->value       = value;
        node->next        = nullptr;
        if (tail == nullptr && head == nullptr) {
            tail = head = node;
        } else {
            tail->next = node;
            tail       = tail->next;
        }
    }

    T pop() {
        if (empty()) __asm__ volatile(".word 0xffffffff");
        struct Node* item = head;
        T value           = item->value;
        head              = head->next;
        if (head == nullptr) tail = nullptr;
        Memory::free(item, sizeof(Node));
        return value;
    }
};

#endif
