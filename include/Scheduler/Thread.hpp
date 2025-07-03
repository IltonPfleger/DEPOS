#pragma once

#include <CPU.hpp>
#include <IO/Logger.hpp>
#include <Machine.hpp>
#include <Memory.hpp>
#include <Scheduler/Queue.hpp>

struct Thread {
    enum Priority { IDLE = 0, LOW, NORMAL, HIGH };
    enum State { RUNNING, READY, WAITING, FINISHED };
    typedef Stack<Thread *> Queue;

    Thread(int (*)(void *), void *, Priority);
    ~Thread();
    uintptr_t stack;
    struct CPU::Context *context;
    struct Thread *joining;
    Queue *waiting;
    enum State state;
    enum Priority priority;

    static void save(CPU::Context *);
    static void join(Thread *);
    static void exit();
    static void init();
    static void stop();
    static void sleep(Queue *);
    static void wakeup(Queue *);
    static void yield();
    static void timer_handler();
};
