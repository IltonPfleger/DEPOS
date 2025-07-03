#pragma once

#include <IO/Logger.hpp>
#include <Meta.hpp>
#include <Settings.hpp>

#define ERROR(expr, message)                      \
    if constexpr (Settings::IO::Enable::ASSERT) { \
        _ERROR(expr, message);                    \
    }

inline void _ERROR(bool cond, const char *message) {
    if (cond) {
        Logger::log("ERROR: ");
        Logger::log(message);
        while (1);
    }
}
