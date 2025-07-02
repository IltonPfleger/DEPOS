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
        for (int i = P::COUNT - 1; i >= 0; i--) {
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

    Node *priorities[P::COUNT];
};

namespace Thread {
    enum Priority { IDLE, LOW, NORMAL, HIGH, COUNT };
    enum State { RUNNING, READY, WAITING, FINISHED };
    struct Thread {
        //Thread(int (*)(void *), void *, Priority);
        //~Thread();
        uintptr_t stack;
        struct CPU::Context *context;
        struct Thread *joining;
        enum State state;
        enum Priority priority;
    };
    typedef Queue<Thread, Priority> Queue;

    void save(CPU::Context *);
    void join(Thread *);
    void create(Thread *, int (*)(void *), void *, Priority);
    void exit();
    void init();
    void sleep(Queue *);
    void wakeup(Queue *);
    void yield();
    void timer_handler();
}  // namespace Thread
