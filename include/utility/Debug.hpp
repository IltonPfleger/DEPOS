#pragma once

#include <utility/Console.hpp>

static constexpr const char *TracePrefix      = "[TRACE] ";
static constexpr const char *ErrorPrefix      = "\n[ERROR] ";
static constexpr const char *ExpressionPrefix = "Expression: ";
static constexpr const char *MessagePrefix    = "Message: ";
static constexpr const char *ReturnLabel      = "return=";
static constexpr const char *TraceInEnd       = ") {";

#define assert(expression, ...)                                                                                        \
    if constexpr (DEPOS::Traits<DEPOS::Debug>::Error) {                                                                \
        if (!(expression)) {                                                                                           \
            DEPOS::Console::panic();                                                                                   \
            DEPOS::Console::println("[ASSERT] ", __PRETTY_FUNCTION__);                                                 \
            DEPOS::Console::println(#expression);                                                                      \
            __VA_OPT__(DEPOS::Console::println(__VA_ARGS__);)                                                          \
            for (;;)                                                                                                   \
                ;                                                                                                      \
        }                                                                                                              \
    }

#define __LOCATION__ __PRETTY_FUNCTION__
#define ERROR(expr, ...)                                                                                               \
    if constexpr (DEPOS::Traits<DEPOS::Debug>::Error) {                                                                \
        if (expr) {                                                                                                    \
            DEPOS::Console::panic();                                                                                   \
            DEPOS::Console::println(ErrorPrefix, __LOCATION__);                                                        \
            DEPOS::Console::println(ExpressionPrefix, #expr);                                                          \
            __VA_OPT__(DEPOS::Console::println(MessagePrefix, __VA_ARGS__);)                                           \
            for (;;)                                                                                                   \
                ;                                                                                                      \
        }                                                                                                              \
    }

// ********** Traces **********

#define Trace(...)                                                                                                     \
    if constexpr (DEPOS::Traits<DEPOS::Debug>::Trace) {                                                                \
        __VA_OPT__(DEPOS::Console::print(__VA_ARGS__);)                                                                \
    }

#define TraceLocation() Trace(__LOCATION__)

#define TraceIn(...)                                                                                                   \
    if constexpr (DEPOS::Traits<DEPOS::Debug>::Trace) {                                                                \
        TraceLocation();                                                                                               \
        DEPOS::Console::print('(');                                                                                    \
        __VA_OPT__(int n = 0;                                                                                          \
                   [&](auto &&...args) { ((DEPOS::Console::print(n++ ? ',' : '\0', args)), ...); }(__VA_ARGS__);)      \
        DEPOS::Console::println(TraceInEnd);                                                                           \
    }

#define TraceOut(...)                                                                                                  \
    if constexpr (DEPOS::Traits<DEPOS::Debug>::Trace) {                                                                \
        __VA_OPT__(DEPOS::Console::print(ReturnLabel); int n = 0;                                                      \
                   [&](auto &&...args) { ((DEPOS::Console::print(n++ ? ',' : '\0', args)), ...); }(__VA_ARGS__);       \
                   DEPOS::Console::print('\n');)                                                                       \
        DEPOS::Console::print(__func__);                                                                               \
        DEPOS::Console::println('}');                                                                                  \
    }
