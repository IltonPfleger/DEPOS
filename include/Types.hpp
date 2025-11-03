#pragma once
#include <Meta.hpp>

typedef int int32_t;
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef Meta::TypeSelector<sizeof(void *) == 8, long long, int>::Result intmax_t;
typedef Meta::TypeSelector<sizeof(void *) == 8, long long, int>::Result intptr_t;
typedef Meta::TypeSelector<sizeof(void *) == 8, unsigned long long, unsigned>::Result uintptr_t;
typedef Meta::TypeSelector<sizeof(void *) == 8, unsigned long long, unsigned>::Result uintmax_t;
typedef Meta::TypeSelector<sizeof(void *) == 8, unsigned long long, void>::Result uint64_t;
typedef uint32_t size_t;
typedef uintmax_t Microsecond;
typedef uintmax_t Tick;
