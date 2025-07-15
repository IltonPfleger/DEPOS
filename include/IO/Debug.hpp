#pragma once

#include <CPU.hpp>
#include <IO/Logger.hpp>
#include <Meta.hpp>
#include <Traits.hpp>

#define ERROR(expr, ...)                    \
    if constexpr (Traits<Debug>::ERROR) {   \
        if (expr) {                         \
            CPU::Interrupt::disable();      \
            Logger::println("\nERROR: ");   \
            Logger::println(__VA_ARGS__); \
            for (;;);                       \
        }                                   \
    }
