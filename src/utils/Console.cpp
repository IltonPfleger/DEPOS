#include <machine/Machine.hpp>
#include <utils/Console.hpp>

static constexpr char HEX[] = "0123456789ABCDEF";

void Console::init() { IO::init(); }
void Console::put(char c) { IO::put(c); }

void Console::print(void *p) {
    auto addr = reinterpret_cast<unsigned long>(p);
    put('0');
    put('x');
    for (int i = (sizeof(void *) * 2) - 1; i >= 0; i--) {
        put(HEX[(addr >> (i * 4)) & 0xF]);
    }
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
