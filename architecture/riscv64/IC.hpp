#pragma once

#include <architecture/riscv64/ContextFrame.hpp>
#include <architecture/riscv64/PLIC.hpp>
#include <utility/Debug.hpp>

namespace DEPOS {

class IC : Traits<PLIC> {
    using ID              = size_t;
    using ExternalHandler = void (*)(ID);

  private:
    static void doNothing(size_t) {}

  public:
    static void onTrap(ID, ContextFrame *) {
        auto id = PLIC::claim();
        handlers_[id](id);
        PLIC::complete(id);
    }

    static void install(ID id, ExternalHandler handler) {
        ERROR(id >= NumberOfInterruptions);
        handlers_[id] = handler;
        PLIC::priority(id, 1);
        PLIC::enable(id);
    }

  private:
    static constinit inline ExternalHandler handlers_[NumberOfInterruptions] = {doNothing};
};

} // namespace DEPOS
