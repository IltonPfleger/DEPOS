#pragma once
#include <Meta.hpp>
#include <Variadic.hpp>

static inline constexpr char HEX[] = "0123456789ABCDEF";

struct Console {
    static void init();
    static void put(char);

  public:
    template <Meta::INTEGRAL T> static void print(T i) {
        char buffer[64];
        int position = 0;

        if constexpr (Meta::SIGNED<T>::Result) {
            if (i < 0) {
                Console::put('-');
                i = -i;
            }
        }

        do {
            buffer[position++] = '0' + (i % 10);
            i /= 10;
        } while (i > 0);
        buffer[position] = '\0';
        print(buffer);
    }

    static void print(void *p) {
        auto addr = reinterpret_cast<unsigned long>(p);
        Console::put('0');
        Console::put('x');
        for (int i = (sizeof(void *) * 2) - 1; i >= 0; i--) {
            Console::put(HEX[(addr >> (i * 4)) & 0xF]);
        }
    }
    static void print(char s) { Console::put(s); };
    static void print(const char *s) {
        while (*s)
            Console::put(*s++);
    };

    template <typename T, typename U, typename... Rest> static void print(T &&first, U &&second, Rest &&...rest) {
        print(first);
        print(second);
        (print(rest), ...);
    }

    static void println(const char *, ...);

    // class Stream {
    //    using Manipulator = Stream &(*)(Stream &);

    // public:
    //         inline Stream &operator<<(Manipulator m) { return m(*this); }
    //
    //         inline Stream &operator<<(char c) {
    //             Console::put(c);
    //             return *this;
    //         }
    //
    //         inline Stream &operator<<(const char *str) {
    //             while (*str)
    //                 Console::put(*str++);
    //             return *this;
    //         }
    //
    //         template <Meta::INTEGRAL T> inline Stream &operator<<(T value) {
    //             char buffer[32];
    //             int position = 0;
    //
    //             if constexpr (Meta::SIGNED<T>::Result) {
    //                 if (value < 0) {
    //                     Console::put('-');
    //                     value = -value;
    //                 }
    //             }
    //
    //             do {
    //                 buffer[position++] = '0' + (value % 10);
    //                 value /= 10;
    //             } while (value > 0);
    //
    //             while (position--)
    //                 Console::put(buffer[position]);
    //             return *this;
    //         }
    //
    //         inline Stream &operator<<(const void *pointer) {
    //             auto addr = reinterpret_cast<unsigned long>(pointer);
    //             Console::put('0');
    //             Console::put('x');
    //             for (int i = (sizeof(void *) * 2) - 1; i >= 0; i--) {
    //                 Console::put(HEX[(addr >> (i * 4)) & 0xF]);
    //             }
    //             return *this;
    //         }
    //
    //         static Stream &endl(Stream &s) {
    //             put('\n');
    //             return s;
    //         }
    //     };
    //
    //     template <typename... Args> static void cprintln(Args &&...args) {
    //         bool first = true;
    //         ((Console::out << (first ? (first = false, "") : ", ") << args), ...);
    //     }
    //

    // public:
    // static inline Stream out;
};
