#include <machine/Machine.hpp>
#include <utils/Console.hpp>

void Console::init() { Machine::IO::init(); }
void Console::put(char c) { Machine::IO::put(c); }
void Console::println(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    while (*fmt) {
        if (*fmt == '%') {
            ++fmt;
            switch (*fmt) {
            case 'c': {
                char c = static_cast<char>(va_arg(args, int));
                out << c;
                break;
            }
            case 's': {
                const char *s = va_arg(args, const char *);
                out << s;
                break;
            }
            case 'd': {
                int i = va_arg(args, int);
                out << i;
                break;
            }
            case 'u': {
                unsigned int u = va_arg(args, unsigned int);
                out << u;
                break;
            }
            case 'p': {
                void *p = va_arg(args, void *);
                out << p;
                break;
            }
            case '%': {
                out << '%';
                break;
            }
            default:
                out << '%' << *fmt;
                break;
            }
        } else {
            out << *fmt;
        }
        ++fmt;
    }
    va_end(args);
}
