#ifndef __SPIN_HEADER__
#define __SPIN_HEADER__

class Spin {
  public:
    void acquire();
    void release();

  private:
    volatile bool m_locked = false;
};

#endif
