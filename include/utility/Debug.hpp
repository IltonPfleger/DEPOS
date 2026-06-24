#pragma once

#include <utility/Console.hpp>

static constexpr const char *TracePrefix      = "[TRACE] ";
static constexpr const char *ErrorPrefix      = "\n[ERROR] ";
static constexpr const char *ExpressionPrefix = "Expression: ";
static constexpr const char *MessagePrefix    = "Message: ";
static constexpr const char *ReturnLabel      = "return=";
static constexpr const char *TraceInEnd       = ") {";

#define assert(expression, ...)                                                                                        \
    if constexpr (QUARK::Traits<QUARK::Debug>::Error) {                                                                \
        if (!(expression)) [[unlikely]] {                                                                              \
            QUARK::Console::panic();                                                                                   \
            QUARK::Console::println("\n[ASSERT] ", __PRETTY_FUNCTION__);                                               \
            QUARK::Console::println(#expression);                                                                      \
            __VA_OPT__(QUARK::Console::println(__VA_ARGS__);)                                                          \
            for (;;)                                                                                                   \
                ;                                                                                                      \
        }                                                                                                              \
    }

#define __LOCATION__ __PRETTY_FUNCTION__
#define ERROR(expr, ...)                                                                                               \
    if constexpr (QUARK::Traits<QUARK::Debug>::Error) {                                                                \
        if (expr) {                                                                                                    \
            QUARK::Console::panic();                                                                                   \
            QUARK::Console::println(ErrorPrefix, __LOCATION__);                                                        \
            QUARK::Console::println(ExpressionPrefix, #expr);                                                          \
            __VA_OPT__(QUARK::Console::println(MessagePrefix, __VA_ARGS__);)                                           \
            for (;;)                                                                                                   \
                ;                                                                                                      \
        }                                                                                                              \
    }

// ********** Traces **********

#define Trace(...)                                                                                                     \
    if constexpr (QUARK::Traits<QUARK::Debug>::Trace) {                                                                \
        __VA_OPT__(QUARK::Console::print(__VA_ARGS__);)                                                                \
    }

#define TraceLocation() Trace(__LOCATION__)

#define TraceIn(...)                                                                                                   \
    if constexpr (QUARK::Traits<QUARK::Debug>::Trace) {                                                                \
        TraceLocation();                                                                                               \
        QUARK::Console::print('(');                                                                                    \
        __VA_OPT__(int n = 0;                                                                                          \
                   [&](auto &&...args) { ((QUARK::Console::print(n++ ? ',' : '\0', args)), ...); }(__VA_ARGS__);)      \
        QUARK::Console::println(TraceInEnd);                                                                           \
    }

#define TraceOut(...)                                                                                                  \
    if constexpr (QUARK::Traits<QUARK::Debug>::Trace) {                                                                \
        __VA_OPT__(QUARK::Console::print(ReturnLabel); int n = 0;                                                      \
                   [&](auto &&...args) { ((QUARK::Console::print(n++ ? ',' : '\0', args)), ...); }(__VA_ARGS__);       \
                   QUARK::Console::print('\n');)                                                                       \
        QUARK::Console::print(__func__);                                                                               \
        QUARK::Console::println('}');                                                                                  \
    }
