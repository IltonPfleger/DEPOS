#pragma once

#include <abi/ABI.hpp>

template <typename Response> class Syscall {
  public:
    template <typename... Args> Syscall(ABI::Function call, Args &&...args) {
        static_assert(sizeof...(Args) <= 6);

        long argv[] = {static_cast<long>(args)...};

        register long a0 asm("a0") = sizeof...(Args) > 0 ? argv[0] : 0;
        register long a1 asm("a1") = sizeof...(Args) > 1 ? argv[1] : 0;
        register long a2 asm("a2") = sizeof...(Args) > 2 ? argv[2] : 0;
        register long a3 asm("a3") = sizeof...(Args) > 3 ? argv[3] : 0;
        register long a4 asm("a4") = sizeof...(Args) > 4 ? argv[4] : 0;
        register long a5 asm("a5") = sizeof...(Args) > 5 ? argv[5] : 0;
        register long a7 asm("a7") = static_cast<long>(call);

        asm volatile("ecall" : "+r"(a0) : "r"(a1), "r"(a2), "r"(a3), "r"(a4), "r"(a5), "r"(a7) : "memory");

        response_ = static_cast<Response>(a0);
    }

    operator Response() const { return response_; }

  private:
    Response response_;
};
