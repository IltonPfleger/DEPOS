#pragma once

class Spin {
  public:
    void acquire();
    void release();

  private:
    volatile bool m_locked = false;
};
