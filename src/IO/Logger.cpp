#include <IO/Logger.hpp>
#include <cstdarg>

constexpr char HEX[] = "0123456789ABCDEF";

inline void put(char value) { Traits::Debug::Device::put(value); }

template <typename T>
void printNumber(T value) {
    char buffer[32];
    int position = 0;

    if (value < 0) {
        put('-');
        value = -value;
    }

    do {
        buffer[position++] = '0' + (value % 10);
        value /= 10;
    } while (value > 0);

    while (position--) put(buffer[position]);
}

template <typename T>
static void printHex(T value) {
    put('0');
    put('x');
    for (int i = (sizeof(T) * 2) - 1; i >= 0; i--) {
        put(HEX[(value >> (i * 4)) & 0xF]);
    }
}

void Logger::println(const char* format, ...) {
    va_list args;
    va_start(args, format);
    while (*format) {
        if (*format != '%') {
            put(*format++);
            continue;
        }

        switch (*++format) {
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
                printHex<unsigned int>(va_arg(args, unsigned int));
                break;
            case 'p':
                printHex<uintptr_t>(va_arg(args, uintptr_t));
                break;
            default: {
                put('%');
                put(*format);
                break;
            }
        }
        ++format;
    }
    va_end(args);
}
