#pragma once

namespace rv {
template <int R> static inline void csrw(auto r) { asm volatile("csrw %0, %1" ::"i"(R), "r"(r)); }
template <int R> static inline auto csrr() {
    uint64_t r;
    asm volatile("csrr %0, %1" : "=r"(r) : "i"(R));
    return r;
}
template <int R> static inline void csrs(auto r) { asm volatile("csrs %0, %1" ::"i"(R), "r"(r)); }
template <int R> static inline void csrc(auto r) { asm volatile("csrc %0, %1" ::"i"(R), "r"(r)); }
} // namespace rv
