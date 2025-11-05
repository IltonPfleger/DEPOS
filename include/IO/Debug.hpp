#pragma once

#include <IO/Console.hpp>
#include <Machine.hpp>
#include <Spin.hpp>
#include <Traits.hpp>

template <typename... Args>
inline void __m2fold(Args&&... args) {
    (Console::out << ... << args);
}

#define assert(expr, ...)                                                                         \
    if constexpr (Traits::Debug::ERROR) {                                                         \
        if (!(expr)) {                                                                            \
            Machine::CPU::Interrupt::disable();                                                   \
            Console::out << "\n[ERROR] ";                                                         \
            Console::out << __PRETTY_FUNCTION__;                                                  \
            Console::out << " -> Assertion Failed! -> ";                                          \
            Console::out << "[" << __FILE__ << ":" << __LINE__ << "] {" << Console::Stream::endl; \
            __VA_OPT__(Console::out << "AssertionMessage = ");                                    \
            __VA_OPT__(Console::println(__VA_ARGS__));                                            \
            __VA_OPT__(Console::out << Console::Stream::endl);                                    \
            Console::out << "FailedExpression = " << #expr << Console::Stream::endl;              \
            Console::out << "}" << Console::Stream::endl;                                         \
            for (;;);                                                                             \
        }                                                                                         \
    }

// Console::out << "[ERROR] Assertion Failed -> " << __PRETTY_FUNCTION__ << "{" << #expr << "}\n";
#define ERROR(expr, ...)                                                                       \
    if constexpr (Traits::Debug::ERROR) {                                                      \
        if (expr) {                                                                            \
            Machine::CPU::Interrupt::disable();                                                \
            Console::out << "<" << Machine::CPU::core() << ">" << __PRETTY_FUNCTION__ << "\n"; \
            __m2fold(__VA_ARGS__);                                                             \
            for (;;);                                                                          \
        }                                                                                      \
    }

#define TRACE(...) \
    if constexpr (Traits::Debug::TRACE) __m2fold(__VA_ARGS__);
