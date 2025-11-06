#pragma once

#include <Machine.hpp>
#include <Spin.hpp>
#include <Traits.hpp>
#include <utils/Console.hpp>

#define ERROR(expr, ...)                                                              \
    if constexpr (Traits::Debug::ERROR) {                                             \
        if (expr) {                                                                   \
            Machine::CPU::Interrupt::disable();                                       \
            Console::println("<%d> %s\n", Machine::CPU::core(), __PRETTY_FUNCTION__); \
            __VA_OPT__(Console::println(__VA_ARGS__));                                \
            for (;;);                                                                 \
        }                                                                             \
    }

constexpr const char* TrimPrettyFunction(const char* func) {
    static char buf[128];
    unsigned i = 0;
    while (func[i] && func[i] != '(' && i < sizeof(buf) - 1) {
        buf[i] = func[i];
        i += 1;
    };
    buf[i] = '\0';
    return buf;
}

#define TraceIn(...)                                                                                 \
    if constexpr (Traits::Debug::TRACE) {                                                            \
        Console::println("<%d> %s(", Machine::CPU::core(), TrimPrettyFunction(__PRETTY_FUNCTION__)); \
        __VA_OPT__(Console::cprintln(__VA_ARGS__));                                                  \
        Console::println("){\n");                                                                    \
    }

#define TraceOut(...)                            \
    if constexpr (Traits::Debug::TRACE) {        \
        __VA_OPT__(Console::println("return=")); \
        __VA_OPT__(Console::out << __VA_ARGS__); \
        __VA_OPT__(Console::out << "\n");        \
        Console::println("}\n");                 \
    }
