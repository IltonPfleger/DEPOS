#pragma once

#include <architecture/rv/ic/sbi/Base.hpp>
#include <architecture/rv/ic/sbi/Time.hpp>

namespace rv {
namespace sbi {
class Syscall {
  public:
    static constexpr unsigned int CODE = 9;

    static bool handler(MachineContext *c) {
        bool handle = false;
        switch (c->a7) {
        case Base::EID:
            handle = Base::handler(c);
            break;
        case Time::EID:
            handle = Time::handler(c);
            break;
        }
        c->pc += 4;
        return handle;
    }
};
} // namespace sbi
} // namespace rv
