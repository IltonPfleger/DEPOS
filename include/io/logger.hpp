#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <cstdarg>
#include <definitions.hpp>
#include <io/uart.hpp>

struct Logger {
    static constexpr char HEX[] = "0123456789ABCDEF";
    using Interface             = UART;

    static void init() { Interface::init(); };
    static void put(char value) { Interface::put(value); };

    static void log(const char* format, ...) {
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
                    printNumber<int>(va_arg(args, int));
                    break;
                case 'u':
                    printNumber<unsigned int>(va_arg(args, unsigned int));
                    break;
                case 'x':
                    printHex<int>(va_arg(args, int));
                    break;
                case 'p':
                    printHex<uintptr_t>(va_arg(args, uintptr_t));
                    break;
            }
            format++;
        }
        va_end(args);
    }

    template <typename T>
    static void printNumber(T value) {
        if (value < 0) {
            put('-');
            value *= -1;
        }
        if (value >= 10) printNumber<T>(value / 10);
        put('0' + (value % 10));
    }

    template <typename T>
    static void printHex(T value) {
        put('0');
        put('x');
        for (int i = (sizeof(T) * 2) - 1; i >= 0; i--) {
            put(HEX[(value >> (i * 4)) & 0xF]);
        }
    }
};

#endif
