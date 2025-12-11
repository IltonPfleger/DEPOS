#pragma once
#include <Meta.hpp>
#include <Variadic.hpp>

struct Console {
    static void init();
    static void put(char);

  public:
    template <Meta::Integral T> static void print(T value) {
        if constexpr (Meta::Signed<T>::Result) {
            if (value < 0) {
                put('-');
                value = -value;
            }
        }
        print(static_cast<unsigned long>(value));
    }

    static void print(void *);
    static void print(char);
    static void print(const char *);
    static void print(unsigned long);
    static void println(const char *, ...);

    template <typename T, typename U, typename... Rest> static void print(T &&first, U &&second, Rest &&...rest) {
        print(first);
        print(" ");
        print(second);
        ((print(" "), print(rest)), ...);
    }
};
