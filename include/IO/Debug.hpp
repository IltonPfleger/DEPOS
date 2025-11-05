#pragma once

#include <Machine.hpp>
#include <Spin.hpp>
#include <Traits.hpp>
#include <utils/Console.hpp>

template <typename... Args>
inline void __m2fold(Args&&... args) {
    (Console::out << ... << args);
}

#define ERROR(expr, ...)                                                              \
    if constexpr (Traits::Debug::ERROR) {                                             \
        if (expr) {                                                                   \
            Machine::CPU::Interrupt::disable();                                       \
            Console::println("<%d> %s\n", Machine::CPU::core(), __PRETTY_FUNCTION__); \
            __VA_OPT__(Console::println(__VA_ARGS__));                                \
            for (;;);                                                                 \
        }                                                                             \
    }

#define TRACE(...) \
    if constexpr (Traits::Debug::TRACE) __m2fold(__VA_ARGS__);
