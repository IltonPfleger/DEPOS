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
    enum Priority { HIGH, NORMAL, LOW };
    enum State { RUNNING, READY, WAITING };

    static void exit();
    static void dispatch(Thread*);
    static void yield();
    static void create(Thread*, Entry, Priority);

    static Queue _ready;
    static volatile Thread* _running;
    uintptr_t stack;
    struct CPU::Context context;
    enum State state;
    enum Priority priority;
};

#endif
