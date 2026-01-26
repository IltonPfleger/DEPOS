#pragma once
#include <utils/Console.hpp>

unsigned int inline core() {
    unsigned int core;
    asm("mv %0, tp" : "=r"(core));
    return core;
}

#define ERROR(expr, ...)                                                                                                       \
    if constexpr (Traits<Debug>::Error) {                                                                                      \
        if (expr) {                                                                                                            \
            Console::panic();                                                                                                  \
            Console::println("\n<%d> ", core());                                                                               \
            Console::println("[ERROR] %s\n", __PRETTY_FUNCTION__);                                                             \
            Console::println("Expression: %s\n", #expr);                                                                       \
            __VA_OPT__(Console::println(__VA_ARGS__));                                                                         \
            __VA_OPT__(Console::print('\n'));                                                                                  \
            for (;;) {                                                                                                         \
            }                                                                                                                  \
        }                                                                                                                      \
    }

#define TraceIn(...)                                                                                                           \
    if constexpr (Traits<Debug>::Trace) {                                                                                      \
        Console::println("<%d> ", core());                                                                                   \
        Console::println("%s(", __PRETTY_FUNCTION__);                                                                          \
        __VA_OPT__(Console::print(__VA_ARGS__));                                                                               \
        Console::println("){\n");                                                                                              \
    }

#define TraceOut(...)                                                                                                          \
    if constexpr (Traits<Debug>::Trace) {                                                                                      \
        __VA_OPT__(Console::print("return="));                                                                                 \
        __VA_OPT__(Console::print(__VA_ARGS__));                                                                               \
        __VA_OPT__(Console::print('\n'));                                                                                      \
        Console::println("%s}\n", __func__);                                                                                   \
    }
