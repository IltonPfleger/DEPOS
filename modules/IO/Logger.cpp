module;
#include <cstdarg>
export module Logger;
import Definitions;
import UART;

using Interface = UART;
void put(char value) { Interface::put(value); };
constexpr char HEX[] = "0123456789ABCDEF";

template <typename T>
void printNumber(T value) {
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

export namespace Logger {
    void init() { Interface::init(); };

    void log(const char* format, ...) {
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
};  // namespace Logger
