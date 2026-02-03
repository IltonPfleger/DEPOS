#include <Types.hpp>
extern "C" {
void *memcpy(void *, const void *, size_t);
void *memset(void *, int, size_t);
int strcmp(const char *, const char *);
int memcmp(const void *, const void *, size_t);
unsigned strlen(const char *);
long atol(const char *str);
char *strchr(const char *str, int ch);
}
