#include <io/io.h>
#include <stdarg.h>
#include <stdint.h>

static struct io_interface_s io;

static const char HEX[] = "0123456789ABCDEF";

static void printc(char c) { io.put(c); }

static void printd(int d) {
    if (d < 0) {
        io.put('-');
        d *= -1;
    }
    if (d >= 10) printd(d / 10);
    io.put('0' + (d % 10));
}

static void printu(unsigned int u) {
    if (u >= 10) printu(u / 10);
    io.put('0' + (u % 10));
}

static void printx(int x) {
    io.put('0');
    io.put('x');
    for (int i = 7; i >= 0; i--) {
        io.put(HEX[(x >> (i * 4)) & 0xF]);
    }
}

static void printp(uintptr_t p) {
    io.put('0');
    io.put('x');
    for (int i = sizeof(char*) - 1; i >= 0; i--) {
        io.put(HEX[(p >> (i * 4)) & 0xF]);
    }
}

void io_init(struct io_interface_s _io) { io = _io; }

void io_out(const char* format, ...) {
    va_list args;
    va_start(args, format);
    while (*format) {
        if (*format != '%') {
            printc(*format++);
            continue;
        }
        format++;
        switch (*format) {
            case 'c':
                printc((char)va_arg(args, int));
                break;
            case 'd':
                printd(va_arg(args, int));
                break;
            case 'u':
                printu(va_arg(args, unsigned int));
                break;
            case 'x':
                printx(va_arg(args, uintptr_t));
                break;
            case 'p':
                printp(va_arg(args, uintptr_t));
                     break;
                // default:
                //     kprint_put('%');
                //     kprint_put(*format);
                //     break;
        }
        format++;
    }
    va_end(args);
}
