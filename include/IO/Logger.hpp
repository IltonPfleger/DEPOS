#pragma once
#include <Traits.hpp>

struct Logger {
    static inline void init() { Traits::Debug::Device::init(); }
    static void println(const char *format, ...);
};
