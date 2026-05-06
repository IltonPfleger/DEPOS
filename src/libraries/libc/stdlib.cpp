#include <libraries/libc/stdlib.h>
#include <memory/Heap.hpp>

using namespace DEPOS;

extern "C" void *malloc(size_t size) { return new uint8_t[size]; }
