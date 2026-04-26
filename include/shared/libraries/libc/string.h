#pragma once

#include <types.hpp>

extern "C" {
void *memcpy(void *, const void *, DEPOS::size_t);
void *memset(void *, int, DEPOS::size_t);
int strcmp(const char *, const char *);
int memcmp(const void *, const void *, DEPOS::size_t);
unsigned strlen(const char *);
char *strchr(const char *, int);
}
