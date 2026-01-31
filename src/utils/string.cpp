extern "C" {
void *memcpy(void *dest, const void *src, unsigned long n) {
    unsigned char *d = static_cast<unsigned char *>(dest);
    const unsigned char *s = static_cast<const unsigned char *>(src);
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

void *memset(void *dest, int c, unsigned long n) {
    unsigned char *d = static_cast<unsigned char *>(dest);

    while (n--) {
        *d++ = static_cast<unsigned char>(c);
    }

    return dest;
}

int memcmp(const void *ptr1, const void *ptr2, unsigned long n) {
    const unsigned char *p1 = (const unsigned char *)ptr1;
    const unsigned char *p2 = (const unsigned char *)ptr2;

    for (unsigned long i = 0; i < n; i++) {
        if (p1[i] != p2[i]) {
            return (int)(p1[i] - p2[i]);
        }
    }
    return 0;
}

unsigned long strlen(const char *str) {
    unsigned long n = 0;
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

const char *strchr(const char *str, int ch) {
    while (*str) {
        if (*str == ch) return str;
        ++str;
    }

    if (ch == '\0') return str;

    return nullptr;
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
