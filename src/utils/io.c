#ifndef PRINT_H
#define PRINT_H

#include <definitions.h>
#include <stdarg.h>
#include <utils/uart.h>

static const char HEX[] = "0123456789ABCDEF";

static void printd(int32_t d) {
    if (d < 0) {
        IO_PUT('-');
        d *= -1;
    }
    if (d >= 10) printd(d / 10);
    IO_PUT('0' + (d % 10));
}

static void printx(uint32_t x) {
    IO_PUT('0');
    IO_PUT('x');
    for (int i = 7; i >= 0; i--) {
        IO_PUT(HEX[(x >> (i * 4)) & 0xF]);
    }
}

static void printlx(uint64_t x) {
    IO_PUT('0');
    IO_PUT('x');
    for (int i = 15; i >= 0; i--) {
        IO_PUT(HEX[(x >> (i * 4)) & 0xF]);
    }
}

static void printp(uintptr_t p) {
#if MACHINE_XLEN == 64
    printlx(p);
#else
    printx(p);
#endif
}

void kprint(const char* format, ...) {
    va_list args;
    va_start(args, format);
    while (*format) {
        if (*format != '%') {
            IO_PUT(*format++);
            continue;
        }
        format++;
        switch (*format) {
            case 'c':
                IO_PUT((char)va_arg(args, int32_t));
                break;
            case 'd':
                printd(va_arg(args, int32_t));
                break;
            case 'x':
                printx(va_arg(args, uint32_t));
                break;
            case 'p':
                printp(va_arg(args, uintptr_t));
                break;
            default:
                IO_PUT('%');
                IO_PUT(*format);
                break;
        }
        format++;
    }
    va_end(args);
}

#endif
