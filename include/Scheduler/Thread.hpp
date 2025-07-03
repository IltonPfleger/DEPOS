#pragma once

#include <CPU.hpp>
#include <IO/Logger.hpp>
#include <Machine.hpp>
#include <Memory.hpp>
#include <Scheduler/Queue.hpp>

namespace Thread {
    enum Priority { IDLE = 0, LOW, NORMAL, HIGH };
    enum State { RUNNING, READY, WAITING, FINISHED };
    struct Thread;
    typedef Stack<Thread *> Queue;

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
