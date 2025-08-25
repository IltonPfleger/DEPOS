namespace Timer {
    void handler(unsigned int core);
}

class MIC {
   public:
    class Syscall {
       public:
        enum Type { TIMER };
        static void handler(Type);
    };

    static void error();
    static void handler(void*);
    __attribute__((naked, aligned(4))) static void entry();
};

class SIC {
   public:
    static void error();
    static void handler(void*);
    __attribute__((naked, aligned(4))) static void entry();
};
