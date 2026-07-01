#include <Thread.hpp>
#include <architecture/CPU.hpp>
#include <machine/Machine.hpp>
#include <utility/Console.hpp>

namespace QUARK {

void Console::print(char c) {
    if (panicked()) return;

    Device *device = Device::instance();

    if (column_ >= Traits<Console>::Columns || c == '\n') {
        device->putc('\r');
        column_ = 0;
    }

    column_++;

    device->putc(c);
}

void Console::panic() { CPU::Atomic::cas(panic_, 0, Thread::running()); }

bool Console::panicked() { return (panic_ && panic_ != Thread::running()); }

} // namespace QUARK
