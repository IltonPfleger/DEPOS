#pragma once

#include <abi/ABI.hpp>
#include <architecture/Syscall.hpp>

namespace QUARK::ABI {

class Semaphore {
  public:
    Semaphore(int value = 0) { handler_ = Syscall<void *>(ABI::Function::ABI_SEMAPHORE_CONSTRUCTOR, value); }
    ~Semaphore() { Syscall<void>(ABI::Function::ABI_SEMAPHORE_DESTRUCTOR, handler_); }
    void p() { Syscall<void>(ABI::Function::ABI_SEMAPHORE_P, handler_); }
    void v() { Syscall<void>(ABI::Function::ABI_SEMAPHORE_V, handler_); }

  private:
    void *handler_;
};

}; // namespace QUARK::ABI
