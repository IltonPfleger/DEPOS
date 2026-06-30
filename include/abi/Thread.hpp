#pragma once

#include <abi/ABI.hpp>
#include <architecture/Syscall.hpp>

namespace QUARK {

class Thread {
    template <typename... Args> Thread(Args &&...args) {
        handler_ = Syscall<void *>(ABI::Function::ABI_THREAD_CONSTRUCTOR, static_cast<Args &&>(args)...);
    }

    ~Thread() { Syscall<void>(ABI::Function::ABI_THREAD_DESTRUCTOR, handler_); }

  private:
    void *handler_;
};

}; // namespace QUARK
