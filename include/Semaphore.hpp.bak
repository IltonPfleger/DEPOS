#pragma once
#include <Spin.hpp>
#include <Thread.hpp>

class Semaphore {
    int _value = 1;
    Thread::Queue waiting;
    Spin _lock;

   public:
    void p() {
        _lock.lock();
        if (_value-- < 1)
            Thread::sleep(waiting, _lock);
        else
            _lock.unlock();
    }

    void v() {
        _lock.lock();
        if (_value++ < 0) Thread::wakeup(waiting);
        _lock.unlock();
    }
};
