#pragma once

#include <IO/Logger.hpp>
#include <Meta.hpp>
#include <Traits.hpp>

#define ERROR(expr, message, ...)                \
    if constexpr (Traits<Debug>::ERROR) {        \
        if (expr) {                              \
            Logger::log("ERROR: ");              \
            Logger::log(message, ##__VA_ARGS__); \
        }                                        \
    }
