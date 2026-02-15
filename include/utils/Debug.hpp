#pragma once
#include <utils/Console.hpp>
#include <utils/string.hpp>

#define ERROR(expr, ...)                                                                                                       \
    if constexpr (Traits<Debug>::Error) {                                                                                      \
        if (expr) {                                                                                                            \
            Console::cout << "\n[ERROR] " << __PRETTY_FUNCTION__ << "\nExpression: " << #expr << "\n";                         \
            __VA_OPT__([&](auto &&...args) { ((Console::cout << args), ...); }(__VA_ARGS__);)                                  \
            for (;;)                                                                                                           \
                ;                                                                                                              \
        }                                                                                                                      \
    }

#define Trace(...)                                                                                                             \
    if constexpr (Traits<Debug>::Trace) {                                                                                      \
        __VA_OPT__([&](auto &&...args) { ((Console::cout << args), ...); }(__VA_ARGS__);)                                      \
        __VA_OPT__(Console::cout << Console::endl;)                                                                            \
    }

#define TraceIn(...)                                                                                                           \
    if constexpr (Traits<Debug>::Trace) {                                                                                      \
        Console::cout << __PRETTY_FUNCTION__ << "(";                                                                           \
        __VA_OPT__([&](auto &&...args) {                                                                                       \
            int n = 0;                                                                                                         \
            ((Console::cout << (n++ ? ", " : "") << args), ...);                                                               \
        }(__VA_ARGS__);)                                                                                                       \
        Console::cout << ") {" << Console::endl;                                                                               \
    }

#define TraceOut(...)                                                                                                          \
    if constexpr (Traits<Debug>::Trace) {                                                                                      \
        __VA_OPT__(Console::cout << "return=";)                                                                                \
        __VA_OPT__([&](auto &&...args) {                                                                                       \
            int n = 0;                                                                                                         \
            ((Console::cout << (n++ ? ", " : "") << args), ...);                                                               \
        }(__VA_ARGS__);)                                                                                                       \
        __VA_OPT__(Console::cout << '\n';)                                                                                     \
        Console::cout << __func__ << "}" << Console::endl;                                                                     \
    }
