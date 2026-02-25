#pragma once

#ifdef NDEBUG

#define assert(expr) ((void)0)

#else

#include <utils/Debug.hpp>

#define assert(expr)

#endif
