#pragma once

#include <IO/Logger.hpp>
#include <Machine.hpp>
#include <Spin.hpp>
#include <Traits.hpp>

#define ERROR(expr, ...)                                            \
    if constexpr (Traits::Debug::ERROR) {                           \
        if (expr) {                                                 \
            Machine::CPU::Interrupt::disable();                     \
            Logger::println("\nERROR(%d): ", Machine::CPU::core()); \
            Logger::println(__VA_ARGS__);                           \
            for (;;);                                               \
        }                                                           \
    }

#define TRACE(...)                        \
    if constexpr (Traits::Debug::TRACE) { \
        Logger::println(__VA_ARGS__);     \
    }
