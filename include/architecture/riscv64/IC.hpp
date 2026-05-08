#pragma once

#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/PLIC.hpp>
#include <utility/Debug.hpp>

namespace DEPOS {

namespace riscv64 {

class IC : Traits<PLIC> {
    using ID              = size_t;
    using ExternalHandler = void (*)(ID);

  public:
    static void onTrap(ID, Context *) {
        auto id = PLIC::claim();
        if (id > 0) {
            s_handlers[id](id);
            PLIC::complete(id);
        }
    }

    static void install(ID id, ExternalHandler handler) {
        ERROR(id >= NumberOfInterruptions);
        s_handlers[id] = handler;
        PLIC::priority(id, 1);
        PLIC::enable(id);
    }

  private:
    static constinit inline ExternalHandler s_handlers[NumberOfInterruptions] = {nullptr};
};

} // namespace riscv64

} // namespace DEPOS
