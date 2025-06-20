#ifndef THREAD_HPP
#define THREAD_HPP

#include <cpu.hpp>
#include <definitions.hpp>
#include <memory.hpp>

template <typename T>
struct Queue {
    struct Element {
        unsigned int rank;
        Element* previous;
        Element* next;
        T data;
    };

    void insert(T data, unsigned int rank) {
        Element* item  = new (HEAP) Element;
        item->data     = data;
        item->next     = nullptr;
        item->previous = nullptr;
        item->rank     = rank;

        if (head == nullptr) {
            head = tail = item;
        };

        Element* current = head;
        while (current != nullptr && current->rank < rank) {
            current = current->next;
        }
        if (current == head) {
            item->next     = head;
            head->previous = item;
            head           = item;
        } else if (current == nullptr) {
            tail->next     = item;
            item->previous = tail;
            tail           = item;
        } else {
            item->next              = current;
            item->previous          = current->previous;
            current->previous->next = item;
            current->previous       = item;
        }
    }

    Element* head;
    Element* tail;
    Memory::Heap HEAP;
};

struct Thread {
    typedef int (*ThreadEntry)(void*);
    enum Priority {
        HIGH = 0,
        NORMAL,
        LOW,
        LAST,
    };
    enum State {
        RUNNING,
        READY,
        WAITING,
    };

    static void exit();
    static void dispatch(Thread*);
    static void yield();
    static void create(Thread*, ThreadEntry, Priority);

   private:
    static Queue<Thread*> ready;
    static Queue<Thread*> waiting;

   private:
    uintptr_t stack;
    struct CPU::Context context;
    enum State state;
};

#endif
