#pragma once

class Spin {
  public:
    void acquire();
    void release();
    void lock();
    void unlock();

  private:
    volatile bool locked = false;
    volatile bool interrupts = false;
};
