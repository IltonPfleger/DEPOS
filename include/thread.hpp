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

    typedef int (*Entry)(void*);
    enum Priority { HIGH, NORMAL, LOW, IDLE };
    enum State { RUNNING, READY, WAITING, FINISHED };

    static void exit();
    static void init();
    static void dispatch(Thread*);
    static void yield();
    static void reschedule();
    static void create(Thread*, Entry, Priority);
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
