class Atomic {
  public:
    template <typename T> static void wait(T &value) {
        while (!__atomic_load_n(&value, __ATOMIC_SEQ_CST))
            ;
    }

    template <typename T> static T clear(T &value) {
        return __atomic_exchange_n(&value, 0, __ATOMIC_SEQ_CST);
    }
};
