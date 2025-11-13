void *memcpy(void *dest, const void *src, unsigned long n) {
    unsigned char *d       = static_cast<unsigned char *>(dest);
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
