extern "C" {
int __ctzdi2(unsigned long x) {
    if (x == 0) return 64;
    int n = 0;
    if ((x & 0xFFFFFFFFULL) == 0) {
        n += 32;
        x >>= 32;
    }
    if ((x & 0xFFFFULL) == 0) {
        n += 16;
        x >>= 16;
    }
    if ((x & 0x00FFULL) == 0) {
        n += 8;
        x >>= 8;
    }
    if ((x & 0x000FULL) == 0) {
        n += 4;
        x >>= 4;
    }
    if ((x & 0x0003ULL) == 0) {
        n += 2;
        x >>= 2;
    }
    if ((x & 0x0001ULL) == 0) {
        n += 1;
    }
    return n;
}

int __clzdi2(unsigned long x) {
    if (x == 0) return 64;

    int n = 0;
    if ((x & 0xFFFFFFFF00000000ULL) == 0) {
        n += 32;
        x <<= 32;
    }

    if ((x & 0xFFFF000000000000ULL) == 0) {
        n += 16;
        x <<= 16;
    }

    if ((x & 0xFF00000000000000ULL) == 0) {
        n += 8;
        x <<= 8;
    }
    if ((x & 0xF000000000000000ULL) == 0) {
        n += 4;
        x <<= 4;
    }
    if ((x & 0xC000000000000000ULL) == 0) {
        n += 2;
        x <<= 2;
    }
    if ((x & 0x8000000000000000ULL) == 0) {
        n += 1;
    }

    return n;
}
}
