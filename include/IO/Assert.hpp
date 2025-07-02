#pragma once

#include <IO/Logger.hpp>
#include <Meta.hpp>
#include <Settings.hpp>

#define assert(expr, message)                     \
    if constexpr (Settings::IO::Enable::ASSERT) { \
        _assert(expr, message);                   \
    }

inline void _assert(bool cond, char message[]) {
    if (!cond) {
        Logger::log(" *** ASSERTION FAILURE *** \n");
        Logger::log(message);
        while (1);
    }
}
