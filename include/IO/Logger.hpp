#pragma once
#include <Spin.hpp>

struct Logger {
    static void init();
    static void println(const char *format, ...);
};
