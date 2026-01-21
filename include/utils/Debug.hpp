#pragma once
#include <utils/Console.hpp>
#define ERROR(expr, ...)                                                                                               \
    if constexpr (Traits<Debug>::Error) {                                                                              \
        if (expr) {                                                                                                    \
            Console::println("[ERROR] %s\n", __PRETTY_FUNCTION__);                                                     \
            Console::println("%s\n", #expr);                                                                           \
            __VA_OPT__(Console::println(__VA_ARGS__));                                                                 \
            __VA_OPT__(Console::println("\n"));                                                                        \
            for (;;) {                                                                                                 \
            }                                                                                                          \
        }                                                                                                              \
    }

#define TraceIn(...)                                                                                                   \
    if constexpr (Traits<Debug>::Trace) {                                                                              \
        Console::println("%s(", __PRETTY_FUNCTION__);                                                                  \
        __VA_OPT__(Console::print(__VA_ARGS__));                                                                       \
        Console::println("){\n");                                                                                      \
    }

#define TraceOut(...)                                                                                                  \
    if constexpr (Traits<Debug>::Trace) {                                                                              \
        __VA_OPT__(Console::print("return="));                                                                         \
        __VA_OPT__(Console::out << __VA_ARGS__);                                                                       \
        __VA_OPT__(Console::out << "\n");                                                                              \
        Console::println("%s}\n", __func__);                                                                           \
    }
