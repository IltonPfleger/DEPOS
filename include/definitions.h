#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define MACHINE_ARCH "RISCV"
#define MACHINE_XLEN 64
#define MACHINE_CPUS 4

#define MACHINE_MEMORY_ORDER 30
#define MACHINE_MEMORY_SIZE (1U << MACHINE_MEMORY_ORDER)
#define MACHINE_MEMORY_PAGE_ORDER 12
#define MACHINE_MEMORY_PAGE_SIZE (1U << MACHINE_MEMORY_PAGE_ORDER)

typedef char int8_t;
typedef short int16_t;
typedef int int32_t;
typedef long int int64_t;

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long int uint64_t;

#if XLEN == 64
typedef int64_t intptr_t;
typedef uint64_t uintptr_t;
#else
typedef int32_t intptr_t;
typedef uint32_t uintptr_t;
#endif

#endif
