#ifndef __synchronizer_hpp
#define __synchronizer_hpp

#include <thread.hpp>

struct Synchronizer {
    Thread::Queue waiting;
    unsigned int value;

    void p();
    void v();
};

#endif
