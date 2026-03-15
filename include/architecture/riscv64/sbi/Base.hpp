#pragma once

#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/sbi/Time.hpp>

namespace DEPOS {

namespace riscv64 {

namespace sbi {

class Base {
  public:
    static constexpr unsigned int EID = 0x10;

    enum {
        GET_SPEC_VERSION           = 0,
        GET_IMPLEMENTATION_ID      = 1,
        GET_IMPLEMENTATION_VERSION = 2,
        PROBE_EXTENSION            = 3,
        GET_MVENDORID              = 4,
        GET_MARCHID                = 5,
        GET_MIMPID                 = 6,
    };

    static bool handler(Context *c) {
        bool handle = true;
        switch (c->a6) {
        case GET_MVENDORID: {
            c->a0 = 0;
            c->a1 = csrr<MachineMode::VENDORID>();
            break;
        }
        case GET_MARCHID: {
            c->a0 = 0;
            c->a1 = csrr<MachineMode::ARCHID>();
            break;
        }
        case GET_MIMPID: {
            c->a0 = 0;
            c->a1 = csrr<MachineMode::IMPID>();
            break;
        }
        case GET_SPEC_VERSION: {
            c->a0 = 0;
            c->a1 = 0x00000003;
            break;
        }
        case GET_IMPLEMENTATION_ID: {
            c->a0 = 0;
            c->a1 = 1;
            break;
        }
        case GET_IMPLEMENTATION_VERSION: {
            c->a0 = 0;
            c->a1 = 0x10007;
            break;
        }
        case PROBE_EXTENSION: {
            c->a1 = 0;
            if (c->a0 == Time::EID) c->a1 = 1;
            c->a0 = 0;
            break;
        }
        default:
            handle = false;
        }
        return handle;
    }
};

} // namespace sbi

} // namespace riscv64

} // namespace DEPOS
