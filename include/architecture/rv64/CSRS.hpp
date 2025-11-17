template <const int R> static void csrw(auto r) {
    asm volatile("csrw %c0, %1" ::"i"(R), "r"(r));
}

template <const int R> static auto csrr() {
    uint64_t r;
    asm volatile("csrr %0, %1" : "=r"(r) : "i"(R));
    return r;
}

template <const int R> static void csrs(auto r) {
    asm volatile("csrs %0, %1" ::"i"(R), "r"(r));
}

template <const int R> static void csrc(auto r) {
    asm volatile("csrc %0, %1" ::"i"(R), "r"(r));
}
