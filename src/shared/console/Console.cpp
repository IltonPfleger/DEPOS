#include <architecture/CPU.hpp>
#include <machine/Machine.hpp>
#include <shared/console/Console.hpp>

namespace DEPOS {

void Console::print(char c) {
    if (panicked()) return;

    Device *device = Device::instance();

    if (c == '\n') {
        device->putc('\r');
        s_column = 0;
    }

    if (s_column >= Traits<Console>::Columns) {
        print('\n');
    }

    s_column++;

    device->putc(c);
}

void Console::panic() {
    // uint8_t volatile P;
    // uintptr_t page = reinterpret_cast<uintptr_t>(&P) & ~(Traits<Memory>::PageSize - 1);
    if (!CPU::Atomic::tsl(s_panic)) s_panic = CPU::id();
}

bool Console::panicked() {
    // uint8_t volatile P;
    // uintptr_t page = reinterpret_cast<uintptr_t>(&P) & ~(Traits<Memory>::PageSize - 1);
    // if (s_panic == page) return true;
    if (s_panic && s_panic != CPU::id()) return true;
    return false;
}

} // namespace DEPOS
