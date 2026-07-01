#pragma once

#include <abi/ABI.hpp>
#include <architecture/Syscall.hpp>

namespace QUARK::ABI {

class Thread {
  public:
    template <typename Function, typename Argment> Thread(Function f, Argment a) {
        handler_ = Syscall<void *>(ABI::Function::ABI_THREAD_CONSTRUCTOR, f, a);
    }

    ~Thread() { Syscall<void>(ABI::Function::ABI_THREAD_DESTRUCTOR, handler_); }

    void join() { Syscall<void>(ABI::Function::ABI_THREAD_JOIN, handler_); }

  private:
    void *handler_;
};

}; // namespace QUARK::ABI
