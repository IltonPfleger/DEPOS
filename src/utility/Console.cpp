#include <architecture/CPU.hpp>
#include <machine/Machine.hpp>
#include <utility/Console.hpp>

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
    if (!CPU::Atomic::tsl(s_panic)) s_panic = CPU::id();
}

bool Console::panicked() { return (s_panic && s_panic != CPU::id()); }

} // namespace DEPOS
