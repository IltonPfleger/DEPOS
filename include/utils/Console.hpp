#pragma once

#include <Meta.hpp>
#include <Variadic.hpp>

struct Console {
    static void init();
    static void put(char);

    class Stream {
        using Manipulator = Stream &(*)(Stream &);

      public:
        Stream &operator<<(Manipulator m) { return m(*this); }

        Stream &operator<<(char c) {
            Console::put(c);
            return *this;
        }

        Stream &operator<<(const char *str) {
            if (!str)
                return *this;

            while (*str)
                Console::put(*str++);
            return *this;
        }

        Stream &operator<<(unsigned long value) {
            char buffer[32];
            int pos = 0;

            do {
                buffer[pos++] = char('0' + (value % 10));
                value /= 10;
            } while (value);

            while (pos--)
                Console::put(buffer[pos]);

            return *this;
        }

        template <Meta::Integral T> Stream &operator<<(T value) {
            if constexpr (Meta::Signed<T>::Result) {
                if (value < 0) {
                    Console::put('-');
                    value = -value;
                }
            }
            return *this << static_cast<unsigned long>(value);
        }

        Stream &operator<<(const void *ptr) {
            auto hex = reinterpret_cast<unsigned long>(ptr);
            put('0');
            put('x');
            bool started = false;

            for (int i = (sizeof(hex) * 8) - 4; i >= 0; i -= 4) {
                unsigned int current = (hex >> i) & 0xF;
                if (current != 0 || started) {
                    put(current < 10 ? current + '0' : current - 10 + 'a');
                    started = true;
                }
            }
            if (!started) {
                put('0');
            }
            return *this;
        }

        static Stream &endl(Stream &s) {
            Console::put('\n');
            return s;
        }
    };
    template <typename... Args> static void print(Args &&...args) {
        bool first = true;
        ((Console::out << (first ? (first = false, "") : ", ") << static_cast<Args &&>(args)), ...);
    }

    static void println(const char *, ...);

  public:
    static inline Stream out{};
};
