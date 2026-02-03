#include <memory/Memory.hpp>
#include <memory/MemoryMap.hpp>

class BSS {
  public:
    static void init() {
        for (auto i = Memory::virt2phys(__kmm.bss.start); i != Memory::virt2phys(__kmm.bss.end); i++) {
            *reinterpret_cast<unsigned char *>(i) = 0;
        }
    }
};
