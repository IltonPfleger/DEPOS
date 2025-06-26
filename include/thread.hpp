#ifndef THREAD_HPP
#define THREAD_HPP

#include <cpu.hpp>
#include <definitions.hpp>
#include <memory.hpp>

struct Thread {
    struct Queue {
        struct Node {
            Node* next;
            Thread* value;
        };

        void put(Thread*);
        Thread* get();

        Memory::Heap HEAP;
        Node* head;
    };

    enum Priority { HIGH, NORMAL, LOW, IDLE };
    enum State { RUNNING, READY, WAITING, FINISHED };

    static void exit();
    static void init();
    static void dispatch(Thread *, Thread*);
    static void sleep(Queue*);
    static void wakeup(Queue*);
    static void yield();
    static void reschedule();
    static void create(Thread*, int (*)(void*), void*, Priority);
    static void join(Thread*);
    static int idle(void*);

    static Queue _ready;
    static volatile Thread* _running;

    uintptr_t stack;
    struct CPU::Context* context;
    struct Thread* joining;
    enum State state;
    enum Priority priority;
};

#endif
