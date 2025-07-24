#pragma once
#include <Meta.hpp>

typedef Meta::TypeSelector<sizeof(void *) == 4, long long, int>::Result intptr_t;
typedef Meta::TypeSelector<sizeof(void *) == 4, long long, int>::Result intmax_t;
typedef Meta::TypeSelector<sizeof(void *) == 8, unsigned long long, unsigned>::Result uintptr_t;
typedef Meta::TypeSelector<sizeof(void *) == 8, unsigned long long, unsigned>::Result uintmax_t;
typedef uintmax_t Microsecond;
typedef uintmax_t Tick;
