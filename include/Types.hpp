#pragma once
#include <Meta.hpp>

typedef Meta::TypeSelector<sizeof(void *) == 4, long long, int>::Result intptr_t;
typedef Meta::TypeSelector<sizeof(void *) == 8, unsigned long long, unsigned>::Result uintptr_t;
typedef uintptr_t Microsecond;
