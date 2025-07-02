#pragma once

#include <CPU.hpp>
#include <IO/Logger.hpp>
#include <Machine.hpp>
#include <Memory.hpp>

template <typename T, typename P>
struct Queue {
    struct Node {
        Node *next;
        T *value;
    };

    void put(T *value) {
        P i           = value->priority;
        Node *item    = new (Memory::SYSTEM) Node;
        item->value   = value;
        item->next    = priorities[i];
        priorities[i] = item;
    }

    T *get() {
        for (int i = P::MAX; i >= 0; i--) {
            if (priorities[i]) {
                Node *item    = priorities[i];
                priorities[i] = item->next;
                T *value      = item->value;
                delete item;
                return value;
            }
        }
        return nullptr;
    }

    void remove(T *value) {
        Node *current  = priorities[value->priority];
        Node *previous = nullptr;
        while (current && current->value != value) {
            previous = current;
            current  = current->next;
        }
        if (current->value == value) {
            if (previous)
                previous->next = current->next;
            else
                priorities[value->priority] = current->next;
            delete current;
        }
    }

    Node *priorities[P::MAX + 1];
};

namespace Thread {
    enum Priority { IDLE = 0, LOW, NORMAL, MAX };
    enum State { RUNNING, READY, WAITING, FINISHED };
    struct Thread;
    typedef Queue<Thread, Priority> Queue;
    struct Thread {
        Thread(int (*)(void *), void *, Priority);
        ~Thread();
        uintptr_t stack;
        struct CPU::Context *context;
        struct Thread *joining;
        Queue *waiting;
        enum State state;
        enum Priority priority;
    };

    void save(CPU::Context *);
    void join(Thread *);
    void exit();
    void init();
    void stop();
    void sleep(Queue *);
    void wakeup(Queue *);
    void yield();
    void timer_handler();
}  // namespace Thread
