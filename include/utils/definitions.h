#ifndef DEFINITIONS_H
#define DEFINITIONS_H

#define ARCH "RISCV"
#define XLEN 64
#define CPUS 4

#define MEM_ORDER     30
#define MEM_SIZE      (1U << MEM_ORDER)
#define MEM_MAX_ORDER MEM_ORDER
#define PAGE_ORDER    12
#define PAGE_SIZE     (1U << PAGE_ORDER)

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
