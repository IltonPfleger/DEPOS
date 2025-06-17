#ifndef MEMORY_HPP
#define MEMORY_HPP

#include <definitions.hpp>

struct Memory {
    static void init();
    static void* malloc(unsigned int);
    static void free(void*);
};

#endif
