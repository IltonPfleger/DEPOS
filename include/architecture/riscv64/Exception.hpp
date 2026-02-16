#pragma once

#include <architecture/riscv64/csrs.hpp>
#include <utils/Debug.hpp>

namespace riscv64 {
template <typename Mode = void> class Exception {
  public:
    static void dispatch() {
        char prefix = Mode::PREFIX;
        auto status = reinterpret_cast<void *>(csrr<Mode::STATUS>());
        auto epc = reinterpret_cast<void *>(csrr<Mode::EPC>());
        auto tval = reinterpret_cast<void *>(csrr<Mode::TVAL>());
        auto cause = reinterpret_cast<void *>(csrr<Mode::CAUSE>());
        ERROR(true, "Ohh it's a Trap!\n", prefix, "cause: ", cause, '\n', prefix, "epc: ", epc, '\n', prefix, "tval: ", tval,
              '\n', prefix, "status: ", status, '\n');
    }
};
} // namespace riscv64
