#pragma once
#include <Meta.hpp>
#include <Variadic.hpp>

static inline constexpr char HEX[] = "0123456789ABCDEF";

struct Console {
    static void init();
    static void put(char);

  public:
    template <Meta::Integral T> static void print(T value) {
        char buffer[64];
        int pos = 0;

        if constexpr (Meta::Signed<T>::Result) {
            if (value < 0) {
                put('-');
                value = -value;
            }
        }

        do {
            buffer[pos++] = '0' + (value % 10);
            value /= 10;
        } while (value != 0);

        for (int i = pos - 1; i >= 0; --i)
            put(buffer[i]);
    }

    static void print(void *);
    static void print(char);
    static void print(const char *);
    static void println(const char *, ...);

    template <typename T, typename U, typename... Rest> static void print(T &&first, U &&second, Rest &&...rest) {
        print(first);
        print(second);
        (print(rest), ...);
    }
};
