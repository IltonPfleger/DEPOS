#pragma once

#include <CPU.hpp>
#include <IO/Logger.hpp>
#include <Spin.hpp>
#include <Traits.hpp>

#define ERROR(expr, ...)                                   \
    if constexpr (Traits::Debug::ERROR) {                  \
        if (expr) {                                        \
            CPU::Interrupt::disable();                     \
            Logger::println("\nERROR(%d): ", CPU::core()); \
            Logger::println(__VA_ARGS__);                  \
            for (;;);                                      \
        }                                                  \
    }

#define TRACE(...)                        \
    if constexpr (Traits::Debug::TRACE) { \
        Logger::println("TRACE: ");       \
        Logger::println(__VA_ARGS__);     \
    }
