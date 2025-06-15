#ifndef IO_HPP
#define IO_HPP

#include <stdarg.h>

#include <definitions.hpp>
#include <io/io.hpp>

template <typename IF>
struct IO {
    static constexpr char HEX[] = "0123456789ABCDEF";

    static void init() { IF::init(); };
    static void put(char value) { IF::put(value); };

    static void out(const char* format, ...) {
        va_list args;
        va_start(args, format);
        while (*format) {
            if (*format != '%') {
                put(*format++);
                continue;
            }
            format++;
            switch (*format) {
                case 'c':
                    put((char)va_arg(args, int));
                    break;
                case 'd':
                    _number<int>(va_arg(args, int));
                    break;
                case 'x':
                    _hex<int>(va_arg(args, int));
                    break;
                case 'p':
                    _hex<uintptr_t>(va_arg(args, uintptr_t));
                    break;
                case 'l':
                    format++;
                    switch (*format) {
                        case 'd':
                            _number<intptr_t>(va_arg(args, intptr_t));
                            break;
                        case 'x':
                            _hex<intptr_t>(va_arg(args, intptr_t));
                            break;
                    }
            }
            format++;
        }
        va_end(args);
    }
    template <typename T>
    static void _number(T value) {
        if (value < 0) {
            put('-');
            value *= -1;
        }
        if (value >= 10) _number<T>(value / 10);
        put('0' + (value % 10));
    }

    template <typename T>
    static void _hex(T value) {
        put('0');
        put('x');
        for (int i = (sizeof(T) * 2) - 1; i >= 0; i--) {
            put(HEX[(value >> (i * 4)) & 0xF]);
        }
    }
};

#endif
