#pragma once

#include <IO/Console.hpp>
#include <Machine.hpp>
#include <Spin.hpp>
#include <Traits.hpp>

template <typename... Args>
inline void __m2fold(Args&&... args) {
    (Console::out << ... << args);
}

#define ERROR(expr, ...)                                                 \
    if constexpr (Traits::Debug::ERROR) {                                \
        if (expr) {                                                      \
            Machine::CPU::Interrupt::disable();                          \
            Console::out << "\nERROR(" << Machine::CPU::core() << ")\n"; \
            __m2fold(__VA_ARGS__);                                       \
            for (;;);                                                    \
        }                                                                \
    }

#define TRACE(...) \
    if constexpr (Traits::Debug::TRACE) __m2fold(__VA_ARGS__);
