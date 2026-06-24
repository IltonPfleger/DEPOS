#pragma once

#include <types.hpp>

extern "C" {

void* malloc(QUARK::size_t size);
long atol(const char * str);

}
