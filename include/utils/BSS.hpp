#include <memory/MemoryMap.hpp>

class BSS {
  public:
    static void init() {
        for (auto i = __kmm.bss.start; i != __kmm.bss.end; i++) {
            *reinterpret_cast<unsigned char *>(i) = 0;
        }
    }
};
