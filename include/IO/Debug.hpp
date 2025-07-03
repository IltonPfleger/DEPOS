#pragma once

#include <IO/Logger.hpp>
#include <Meta.hpp>
#include <Settings.hpp>

#define ERROR(expr, message, ...)                 \
    if constexpr (Settings::IO::Enable::ASSERT) { \
        if (expr) {                               \
            Logger::log("ERROR: ");               \
            Logger::log(message, ##__VA_ARGS__);  \
        }                                         \
    }
