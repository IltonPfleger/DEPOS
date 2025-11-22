class Atomic {
  public:
    static auto decf(auto &v) { return __atomic_sub_fetch(&v, 1, __ATOMIC_SEQ_CST); }
    static auto incf(auto &v) { return __atomic_add_fetch(&v, 1, __ATOMIC_SEQ_CST); }
    static auto store(auto &v, auto x) { return __atomic_store_n(&v, x, __ATOMIC_RELEASE); }
    static auto load(auto &v) { return __atomic_load_n(&v, __ATOMIC_CONSUME); }
};
