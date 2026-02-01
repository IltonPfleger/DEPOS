#pragma once
#include <utils/Console.hpp>
#include <utils/string.hpp>

#define ERROR(expr, ...)                                                                                                       \
    if constexpr (Traits<Debug>::Error) {                                                                                      \
        if (expr) {                                                                                                            \
            Console::panic();                                                                                                  \
            Console::println("\n[ERROR] %s\n", __PRETTY_FUNCTION__);                                                           \
            Console::println("Expression: %s\n", #expr);                                                                       \
            __VA_OPT__(Console::println(__VA_ARGS__));                                                                         \
            __VA_OPT__(Console::print('\n'));                                                                                  \
            for (;;) {                                                                                                         \
            }                                                                                                                  \
        }                                                                                                                      \
    }

#define Trace(...)                                                                                                             \
    if constexpr (Traits<Debug>::Trace) {                                                                                      \
        __VA_OPT__(Console::print(__VA_ARGS__));                                                                               \
    }

#define TraceIn(...)                                                                                                           \
    if constexpr (Traits<Debug>::Trace) {                                                                                      \
        char *__name__ = (char *)__PRETTY_FUNCTION__;                                                                          \
        char *__position__ = strchr(__name__, '(');                                                                            \
        if (__position__) *__position__ = '\0';                                                                                \
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
