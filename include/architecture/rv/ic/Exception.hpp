#pragma once

#include <architecture/rv/csrs.hpp>

namespace rv {
template <typename Mode = void> class Exception {
  public:
    static void dispatch() {
        char prefix = Mode::PREFIX;
        auto status = reinterpret_cast<void *>(csrr<Mode::STATUS>());
        auto epc = reinterpret_cast<void *>(csrr<Mode::EPC>());
        auto tval = reinterpret_cast<void *>(csrr<Mode::TVAL>());
        auto cause = reinterpret_cast<void *>(csrr<Mode::CAUSE>());
        ERROR(true,
              "Ohh it's a Trap!\n"
              "%ccause: %d\n"
              "%cepc: %p\n"
              "%ctval: %p\n"
              "%cstatus: %p\n",
              prefix, cause, prefix, epc, prefix, tval, prefix, status);
    }
};
} // namespace rv
