#pragma once
#include <Machine.hpp>
#include <Traits.hpp>

struct Logger {
    static inline void init() { Machine::IO::init(); }
    static void println(const char *format, ...);
};
