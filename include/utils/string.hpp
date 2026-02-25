extern "C" {
void *memcpy(void *, const void *, unsigned long);
void *memset(void *, int, unsigned long);
int strcmp(const char *, const char *);
int memcmp(const void *, const void *, unsigned long);
unsigned strlen(const char *);
long atol(const char *str);
char *strchr(const char *str, int ch);
void *malloc(unsigned long size);
void free(void *ptr);
}
