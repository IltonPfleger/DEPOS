#ifndef __TYPES_HEADER__
#define __TYPES_HEADER__

namespace DEPOS {

typedef int int32_t;
typedef short int16_t;
typedef char int8_t;

typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;

typedef unsigned long size_t;
typedef size_t Hz;
typedef size_t Microsecond;
typedef size_t Nanosecond;
typedef size_t Tick;

} // namespace DEPOS

#include <Meta.hpp>

namespace DEPOS {

typedef Meta::IF<sizeof(long) == 8, long, void>::Result int64_t;
typedef Meta::IF<sizeof(unsigned long) == 8, unsigned long, void>::Result uint64_t;
typedef Meta::IF<sizeof(void *) == 8, long, int>::Result intmax_t;
typedef Meta::IF<sizeof(void *) == 8, unsigned long, unsigned int>::Result uintmax_t;

typedef intmax_t intptr_t;
typedef uintmax_t uintptr_t;

} // namespace DEPOS

#endif
