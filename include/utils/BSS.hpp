#include <memory/MemoryMap.hpp>
#include <utils/string.hpp>

class BSS {
  public:
    static void init() {
        // TraceIn();

        if (__kmm.bss.start != 0) {
            unsigned char *current = reinterpret_cast<unsigned char *>(MMU::virt2phys(__kmm.bss.start));
            memset(current, 0, __kmm.bss.end - __kmm.bss.start);
        }

        // current = reinterpret_cast<unsigned char *>(MMU::virt2phys(__mm.bss.start));
        // end = reinterpret_cast<unsigned char *>(MMU::virt2phys(__mm.bss.end));

        // if (__mm.bss.start != 0) {
        //     for (; current < end; current++) {
        //         *current = 0;
        //     }
        // }
        // TraceOut();
    }
};
