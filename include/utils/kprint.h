#ifndef PRINT_H
#define PRINT_H

#include <stdarg.h>
#include <utils/definitions.h>
#include <utils/uart.h>

#define kprint_init uart_init
#define kprint_put uart_put

static const char HEX[] = "0123456789ABCDEF";

void printd(int32_t d)
{
    if (d < 0) {
        kprint_put('-');
        d *= -1;
    }
    if (d >= 10) printd(d / 10);
    kprint_put('0' + (d % 10));
}

void printx(uint32_t x)
{
    kprint_put('0');
    kprint_put('x');
    for (int i = 7; i >= 0; i--) {
        kprint_put(HEX[(x >> (i * 4)) & 0xF]);
    }
}

void printlx(uint64_t x)
{
    kprint_put('0');
    kprint_put('x');
    for (int i = 15; i >= 0; i--) {
        kprint_put(HEX[(x >> (i * 4)) & 0xF]);
    }
}

void printp(uintptr_t p)
{
    #if XLEN == 64
    printlx(p);
    #else
    printx(p);
    #endif
}

void kprint(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    while (*format) {
        if (*format != '%') {
            kprint_put(*format++);
            continue;
        }
        format++;
        switch (*format) {
            case 'c':
                kprint_put((char)va_arg(args, int32_t));
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
                kprint_put('%');
                kprint_put(*format);
                break;
        }
        format++;
    }
    va_end(args);
}

#endif
