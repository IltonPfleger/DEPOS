#pragma once

extern "C" void (*__init_array_start[])() __attribute__((weak));
extern "C" void (*__init_array_end[])() __attribute__((weak));
class Language {
   public:
    static void init() {
        for (void (**f)() = __init_array_start; f != __init_array_end; ++f) (*f)();
    }
};
