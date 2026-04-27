#ifndef __SYNCHRONIZATION_SPIN_HEADER__
#define __SYNCHRONIZATION_SPIN_HEADER__

class Spin {
  public:
    void acquire();
    void release();

  private:
    volatile bool m_locked = false;
};

#endif
