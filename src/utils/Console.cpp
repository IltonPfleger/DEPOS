#include <machine/Machine.hpp>
#include <utils/Console.hpp>

void Console::init() { IO::init(); }
void Console::put(char c) {
    if (c == '\n')
        IO::put('\r');
    IO::put(c);
}

void Console::print(void *value) {
    auto hex = reinterpret_cast<unsigned long>(value);
    put('0');
    put('x');
    bool started = false;

    for (int i = (sizeof(hex) * 8) - 4; i >= 0; i -= 4) {
        unsigned int current = (hex >> i) & 0xF;
        if (current != 0 || started) {
            put(current < 10 ? current + '0' : current - 10 + 'A');
            started = true;
        }
    }
    if (!started) {
        put('0');
    }
}

void Console::print(unsigned long value) {
    char buffer[64];
    int position = 0;
    do {
        buffer[position++] = '0' + (value % 10);
        value /= 10;
    } while (value != 0);

    for (int i = position - 1; i >= 0; --i)
        put(buffer[i]);
}

void Console::print(char s) { put(s); };
void Console::print(const char *s) {
    while (*s)
        put(*s++);
};

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
                int i = va_arg(args, int);
                print(i);
                break;
            }
            case 'u': {
                unsigned int u = va_arg(args, unsigned int);
                print(u);
                break;
            }
            case 'p': {
                void *p = va_arg(args, void *);
                print(p);
                break;
            }
            case 'x': {
                void *p = va_arg(args, void *);
                print(p);
                break;
            }

            case '%': {
                print('%');
                break;
            }
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
