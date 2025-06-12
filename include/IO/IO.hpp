#ifndef KPRINT_H
#define KPRINT_H

#include <stdarg.h>

template <typename IF>
struct IO {
    static inline void init() { IF::init(); };
    static inline void put(char value) { IF::put(value); };
    static constexpr char HEX[] = "0123456789ABCDEF";

    static void printc(char value) { put(value); }

    static void printd(int32_t value) {
        if (value < 0) {
            put('-');
            value *= -1;
        }
        if (value >= 10) printd(value / 10);
        put('0' + (value % 10));
    }

    static void printx(uint32_t value) {
        put('0');
        put('x');
        for (int i = 7; i >= 0; i--) {
            put(HEX[(value >> (i * 4)) & 0xF]);
        }
    }

    static void printlx(uint64_t value) {
        put('0');
        put('x');
        for (int i = 15; i >= 0; i--) {
            put(HEX[(value >> (i * 4)) & 0xF]);
        }
    }

    static void printp(uint32_t value) { printx(value); }

    static void printp(uint64_t value) { printlx(value); }

    static void print(const char* format, ...) {
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
                    printc((char)va_arg(args, int32_t));
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
            }
            format++;
        }
        va_end(args);
    }
};

// static const char HEX[] = "0123456789ABCDEF";
//
// void printd(int32_t d)
//{
//     if (d < 0) {
//         kprint_put('-');
//         d *= -1;
//     }
//     if (d >= 10) printd(d / 10);
//     kprint_put('0' + (d % 10));
// }
//
// void printx(uint32_t x)
//{
//     kprint_put('0');
//     kprint_put('x');
//     for (int i = 7; i >= 0; i--) {
//         kprint_put(HEX[(x >> (i * 4)) & 0xF]);
//     }
// }
//
// void printlx(uint64_t x)
//{
//     kprint_put('0');
//     kprint_put('x');
//     for (int i = 15; i >= 0; i--) {
//         kprint_put(HEX[(x >> (i * 4)) & 0xF]);
//     }
// }
//
// void printp(uintptr_t p)
//{
//     #if XLEN == 64
//     printlx(p);
//     #else
//     printx(p);
//     #endif
// }
//
// void kprint(const char* format, ...)
//{
//     va_list args;
//     va_start(args, format);
//     while (*format) {
//         if (*format != '%') {
//             kprint_put(*format++);
//             continue;
//         }
//         format++;
//         switch (*format) {
//             case 'c':
//                 kprint_put((char)va_arg(args, int32_t));
//                 break;
//             case 'd':
//                 printd(va_arg(args, int32_t));
//                 break;
//             case 'x':
//                 printx(va_arg(args, uint32_t));
//                 break;
//             case 'p':
//                 printp(va_arg(args, uintptr_t));
//                 break;
//             default:
//                 kprint_put('%');
//                 kprint_put(*format);
//                 break;
//         }
//         format++;
//     }
//     va_end(args);
// }

#endif
