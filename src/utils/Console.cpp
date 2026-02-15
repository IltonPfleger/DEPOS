#include <abstractions/CPU.hpp>
#include <machine/Machine.hpp>
#include <utils/Console.hpp>

void Console::put(char c) {
    if (s_panic != 0 && s_panic != (CPU::id() + 1)) return;

    if (c == '\n') {
        IO::put('\r');
        s_column = 0;
    }

    if (s_column >= Traits<Console>::Columns) {
        Console::put('\n');
    }

    s_column++;

    IO::put(c);
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
