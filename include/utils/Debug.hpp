#pragma once
#include <utils/Console.hpp>
#define ERROR(expr, ...)                                                                                               \
    if constexpr (Traits<Debug>::Error) {                                                                              \
        if (expr) {                                                                                                    \
            CPU::Interruptions::disable();                                                                             \
            Console::println("<%d> [ERROR] %s\n", CPU::id(), __PRETTY_FUNCTION__);                                     \
            __VA_OPT__(Console::println(__VA_ARGS__));                                                                 \
            __VA_OPT__(Console::println("\n"));                                                                        \
            for (;;) {                                                                                                 \
            }                                                                                                          \
        }                                                                                                              \
    }

#define TraceIn(...)                                                                                                   \
    if constexpr (Traits<Debug>::Trace) {                                                                              \
        Console::println("<%d> %s(", CPU::id(), __PRETTY_FUNCTION__);                                                  \
        __VA_OPT__(Console::cprintln(__VA_ARGS__));                                                                    \
        Console::println("){\n");                                                                                      \
    }

#define TraceOut(...)                                                                                                  \
    if constexpr (Traits<Debug>::Trace) {                                                                              \
        __VA_OPT__(Console::println("return="));                                                                       \
        __VA_OPT__(Console::out << __VA_ARGS__);                                                                       \
        __VA_OPT__(Console::out << "\n");                                                                              \
        Console::println("%s}\n", __func__);                                                                           \
    }
