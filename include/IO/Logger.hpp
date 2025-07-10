#pragma once

struct Logger {
    static void init();
    static void println(const char *format, ...);
};
