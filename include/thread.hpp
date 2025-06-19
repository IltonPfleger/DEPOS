#ifndef PROCESS_HPP
#define PROCESS_HPP

#include <cpu.hpp>
#include <definitions.hpp>

struct Thread {
    typedef int (*ThreadFunction)(void*);
    enum Priority {
        LOW,
        NORMAL,
        HIGH,
    };
    enum State {
        RUNNING,
        READY,
        WAITING,
    };

    Thread(ThreadFunction);
    static void exit();

   private:
    struct CPU::Context context;
    enum State state;
};

#endif
