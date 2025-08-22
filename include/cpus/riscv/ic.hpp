
class MIC {
   public:
    static void error();
    static void handler();
    __attribute__((naked, aligned(4))) static void entry();
};

class SIC {};
