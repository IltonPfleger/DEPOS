#include <architecture/CPU.hpp>
#include <machine/Machine.hpp>
#include <utils/Console.hpp>

namespace DEPOS {

void Console::put(char c) {
    if (panicked()) return;

    IO *io = IO::instance();

    if (c == '\n') {
        io->putc('\r');
        s_column = 0;
    }

    if (s_column >= Traits<Console>::Columns) {
        put('\n');
    }

    s_column++;

    io->putc(c);
}

void Console::panic() {
    unsigned char volatile G;
    if (!s_panic) s_panic = (reinterpret_cast<unsigned long>(&G) & ~(Traits<Memory>::StackSize - 1));
}

bool Console::panicked() {
    unsigned char volatile G;
    if (s_panic && s_panic != (reinterpret_cast<unsigned long>(&G) & ~(Traits<Memory>::StackSize - 1))) return true;
    return false;
}

} // namespace DEPOS
