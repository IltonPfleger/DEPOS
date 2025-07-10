#pragma once

#include <IO/Logger.hpp>
#include <Meta.hpp>
#include <Traits.hpp>

#define ERROR(expr, ...)                           \
    if constexpr (Traits<Debug>::ERROR) {          \
        if (expr) {                                \
            Logger::println("ERROR: ", ##__VA_ARGS__); \
        }                                          \
    }
