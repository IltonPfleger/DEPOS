namespace Timer {
    void handler(unsigned int core);
}

class MIC {
    static void error(intmax_t);
    static void handler(void*);

    enum Interrupt { TIMER = 7 };
    enum Exception { SYSCALL = 9 };

   public:
    __attribute__((naked, aligned(4))) static void entry();
};

class SIC {
    static void error(intmax_t);
    static void handler();
    enum Interrupt { TIMER = 5 };

   public:
    __attribute__((naked, aligned(4))) static void entry();
};

class Syscall {
    friend MIC;

   public:
    enum Code { RESET_CLINT_TIMER };
    static void call(Code);

   private:
    static void handler(Code);
};
