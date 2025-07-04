#pragma once

struct Logger {
    static void init();
    static void log(const char* format, ...);
};
