#pragma once

class Spin {
    volatile bool locked        = !LOCKED;
    volatile bool interrupts    = !LOCKED;
    static constexpr int LOCKED = 1;

   public:
    void acquire();

    void release();

    void lock();

    void unlock();
};
