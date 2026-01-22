#include <machine/Machine.hpp>
#include <utils/Console.hpp>

void Console::put(char c) {
    if (c == '\n')
        Machine::IO::put('\r');
    Machine::IO::put(c);
}

void Console::print(char c) { put(c); }

void Console::print(const char *c) {
    while (*c)
        print(*c++);
}

void Console::print(unsigned long lu) {
    char buffer[24];
    unsigned int i = 0;

    do {
        buffer[i++] = char('0' + (lu % 10));
        lu /= 10;
    } while (lu);

    while (i--)
        print(buffer[i]);
}

void Console::println(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    while (*fmt) {
        if (*fmt == '%') {
            ++fmt;
            switch (*fmt) {
            case 'c': {
                char c = static_cast<char>(va_arg(args, int));
                print(c);
                break;
            }
            case 's': {
                const char *s = va_arg(args, const char *);
                print(s);
                break;
            }
            case 'd': {
                int d = va_arg(args, int);
                print(d);
                break;
            }
            case 'u': {
                unsigned int u = va_arg(args, unsigned int);
                print(u);
                break;
            }
            case 'p': {
                print("0x");
                void *p = va_arg(args, void *);
                print(p);
                break;
            }
            case 'x': {
                unsigned long x = va_arg(args, unsigned long);
                print(Hex(x));
                break;
            }
            }
        } else {
            print(*fmt);
        }
        ++fmt;
    }
    va_end(args);
}
