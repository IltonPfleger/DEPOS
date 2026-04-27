#pragma once

#include <shared/console/Console.hpp>

namespace Messages {

static constexpr const char *ErrorPrefix      = "\n[ERROR] ";
static constexpr const char *ExpressionPrefix = "Expression: ";
static constexpr const char *MessagePrefix    = "Message: ";
static constexpr const char *ReturnLabel      = "return=";
static constexpr const char *TraceInEnd       = ") {";

consteval unsigned int MessageSize(const char *string) {
    unsigned int size = 0;
    while (string[size] != '\0' && string[size] != '(') {
        ++size;
    }
    return size;
}

template <unsigned int N> struct Parser {
    char data[N + 1]{};

    consteval Parser(const char *string) {
        for (unsigned int i = 0; i < N; ++i) {
            data[i] = string[i];
        }
        data[N] = '\0';
    }

    constexpr operator const char *() { return data; }
};

} // namespace Messages

#define LOCATION Messages::Parser<Messages::MessageSize(__PRETTY_FUNCTION__)>(__PRETTY_FUNCTION__)

#define ERROR(expr, ...)                                                                                               \
    if constexpr (Traits<Debug>::Error) {                                                                              \
        if (expr) {                                                                                                    \
            Console::panic();                                                                                          \
            Console::println(Messages::ErrorPrefix, __PRETTY_FUNCTION__);                                              \
            Console::println(Messages::ExpressionPrefix, #expr);                                                       \
            __VA_OPT__(Console::println(Messages::MessagePrefix, __VA_ARGS__);)                                        \
            for (;;)                                                                                                   \
                ;                                                                                                      \
        }                                                                                                              \
    }

#define Trace(...)                                                                                                     \
    if constexpr (Traits<Debug>::Trace) {                                                                              \
        __VA_OPT__(Console::print(__VA_ARGS__);)                                                                       \
    }

#define TraceIn(...)                                                                                                   \
    if constexpr (Traits<Debug>::Trace) {                                                                              \
        Console::print(LOCATION);                                                                                      \
        Console::print('(');                                                                                           \
        __VA_OPT__(int n = 0; [&](auto &&...args) { ((Console::print(n++ ? ',' : '\0', args)), ...); }(__VA_ARGS__);)  \
        Console::println(Messages::TraceInEnd);                                                                        \
    }

#define TraceOut(...)                                                                                                  \
    if constexpr (Traits<Debug>::Trace) {                                                                              \
        __VA_OPT__(Console::print(Messages::ReturnLabel); int n = 0;                                                   \
                   [&](auto &&...args) { ((Console::print(n++ ? ',' : '\0', args)), ...); }(__VA_ARGS__);              \
                   Console::print('\n');)                                                                              \
        Console::print(__func__);                                                                                      \
        Console::println('}');                                                                                         \
    }
