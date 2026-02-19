#include <utils/string.hpp>

extern "C" {
void *memcpy(void *destination, const void *source, size_t n) {
    uint8_t *d = static_cast<uint8_t *>(destination);
    const uint8_t *s = static_cast<const uint8_t *>(source);

    while (n--) {
        *d++ = *s++;
    }

    return destination;
}

void *memset(void *dest, int c, size_t n) {
    unsigned char *d = static_cast<unsigned char *>(dest);

    while (n--) {
        *d++ = static_cast<unsigned char>(c);
    }

    return dest;
}

int memcmp(const void *ptr1, const void *ptr2, size_t n) {
    const unsigned char *p1 = (const unsigned char *)ptr1;
    const unsigned char *p2 = (const unsigned char *)ptr2;

    for (size_t i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return (int)(p1[i] - p2[i]);
        }
    }
    return 0;
}

unsigned strlen(const char *str) {
    size_t n = 0;
    while (*str++)
        n++;
    return n;
}

int strcmp(const char *s1, const char *s2) {
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return (unsigned char)(*s1) - (unsigned char)(*s2);
}

char *strchr(const char *str, int ch) {
    while (*str != (char)ch) {
        if (!*str) {
            return nullptr;
        }
        str++;
    }
    return const_cast<char *>(str);
}

long atol(const char *str) {
    long result = 0;
    int sign = 1;

    while (*str == ' ' || *str == '\t' || *str == '\n')
        ++str;

    if (*str == '-') {
        sign = -1;
        ++str;
    } else if (*str == '+') {
        ++str;
    }

    while (*str >= '0' && *str <= '9') {
        result = result * 10 + (*str - '0');
        ++str;
    }

    return result * sign;
}
}
