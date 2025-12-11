#include <memory/MemoryMap.hpp>
class BSS {
  public:
    static void init() {
        unsigned char *current = reinterpret_cast<unsigned char *>(__kmm.bss.start);
        unsigned char *end = reinterpret_cast<unsigned char *>(__kmm.bss.end);
        for (; current < end; current++) {
            *current = 0;
        }

        current = reinterpret_cast<unsigned char *>(__mm.bss.start);
        end = reinterpret_cast<unsigned char *>(__mm.bss.end);
        for (; current < end; current++) {
            *current = 0;
        }
    }
};
