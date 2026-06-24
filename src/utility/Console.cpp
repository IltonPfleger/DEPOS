#include <Thread.hpp>
#include <architecture/CPU.hpp>
#include <machine/Machine.hpp>
#include <utility/Console.hpp>

namespace QUARK {

void Console::print(char c) {
    if (panicked()) return;

    Device *device = Device::instance();

    if (s_column >= Traits<Console>::Columns || c == '\n') {
        device->putc('\r');
        s_column = 0;
    }

    s_column++;

    device->putc(c);
}

void Console::panic() { CPU::Atomic::cas(s_panic, 0, Thread::running()); }

bool Console::panicked() { return (s_panic && s_panic != Thread::running()); }

} // namespace QUARK
