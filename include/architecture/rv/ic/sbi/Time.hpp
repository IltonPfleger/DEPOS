#pragma once

#include <architecture/rv/CLINT.hpp>
#include <architecture/rv/Context.hpp>

namespace rv {
namespace sbi {
class Time {
  public:
    static constexpr unsigned int EID = 'T' << 24 | 'I' << 16 | 'M' << 8 | 'E';

    static bool handler(MachineContext *c) {
        CLINT::write(c->a0);
        c->a0 = 0;
        c->a1 = 0;
        return true;
    }
};
} // namespace sbi
} // namespace rv
