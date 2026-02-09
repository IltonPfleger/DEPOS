// #pragma once
//
// #include <architecture/rv/CLINT.hpp>
// #include <architecture/rv/Context.hpp>
//
// namespace rv {
// class ReducedSBI {
//   public:
//     enum { TIME = 'T' << 24 | 'I' << 16 | 'M' << 8 | 'E' };
//
//     static bool dispatch(MachineContext *c) {
//         uintmax_t mcause = csrr<MachineMode::CAUSE>();
//
//         if (mcause == 9) {
//             if (c->a7 == TIME) {
//                 CLINT::syscall();
//                 c->pc += 4;
//                 return true;
//             }
//         }
//         return false;
//     }
// };
// } // namespace rv
