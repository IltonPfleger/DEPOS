#include <machine/Machine.hpp>
#include <utils/Console.hpp>

void Console::put(char c) {
    if (c == '\n')
        IO::put('\r');
    IO::put(c);
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
            case 'c':
                print(static_cast<char>(va_arg(args, int)));
                break;
            case 's':
                print(va_arg(args, const char *));
                break;
            case 'd':
                print(va_arg(args, int));
                break;
            case 'u':
                print(va_arg(args, unsigned int));
                break;
            case 'p':
                print(va_arg(args, void *));
                break;
            case 'x':
                print(Hex(va_arg(args, unsigned int)));
                break;
            default:
                print('%');
                print(*fmt);
                break;
            }
        } else {
            print(*fmt);
        }
        ++fmt;
    }
    va_end(args);
}
