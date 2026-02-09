// #pragma once
//
// #include <architecture/rv/Context.hpp>
// #include <architecture/rv/Modes.hpp>
// #include <architecture/rv/csrs.hpp>
// #include <architecture/rv/ic/sbi/IllegalInstruction.hpp>
// #include <architecture/rv/ic/sbi/Syscall.hpp>
//
// namespace rv {
// namespace sbi {
// class SBI {
//   public:
//     static bool dispatch(MachineContext *c) {
//         bool handle = false;
//         uintmax_t mcause = csrr<MachineMode::CAUSE>();
//
//         switch (mcause) {
//         case rv::sbi::Syscall::CODE:
//             handle = rv::sbi::Syscall::handler(c);
//             break;
//         case rv::sbi::IllegalInstruction::CODE:
//             handle = rv::sbi::IllegalInstruction::handler(c);
//             break;
//         }
//         return handle;
//     }
// };
// } // namespace sbi
// } // namespace rv
