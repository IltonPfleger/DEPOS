#ifndef __QUARK_MUTEX__
#define __QUARK_MUTEX__

#include <Semaphore.hpp>

namespace QUARK {

class Mutex : Semaphore {
  public:
    constexpr Mutex()
        : Semaphore(1) {}

    void acquire() { p(); }
    void release() { v(); }
};

} // namespace QUARK

#endif
