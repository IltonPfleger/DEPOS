namespace Timer {
    void handler(unsigned int core);
}

class MIC {
    static void error();
    static void handler(void*);

   public:
    __attribute__((naked, aligned(4))) static void entry();
};

class SIC {
    static void error();
    static void handler();

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
