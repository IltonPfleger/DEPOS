#pragma once

namespace ArchitectureCommon {
class Atomic {
  public:
    static auto fdec(auto &v) { return __atomic_fetch_sub(&v, 1, __ATOMIC_SEQ_CST); }
    static auto finc(auto &v) { return __atomic_fetch_add(&v, 1, __ATOMIC_SEQ_CST); }
    static auto store(auto &v, auto x) { return __atomic_store_n(&v, x, __ATOMIC_RELEASE); }
    static auto load(auto &v) { return __atomic_load_n(&v, __ATOMIC_ACQUIRE); }
};
} // namespace ArchitectureCommon
