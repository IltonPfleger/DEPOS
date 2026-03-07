#include <Traits.hpp>
#include <abstractions/VirtualCPU.hpp>
#include <architecture/CPU.hpp>
#include <drivers/virtio/Console.hpp>
#include <machine/Machine.hpp>
#include <utils/Console.hpp>
#include <utils/string.hpp>

using namespace DEPOS;

static constexpr unsigned char EPOS[] = {
#include __STR(__KERNEL__)
};

int main() {
    TraceIn();

    constexpr long MB         = 1024 * 1024;
    constexpr long MemorySize = 128 * MB;

    unsigned char *buffer = reinterpret_cast<unsigned char *>(Memory::alloc(MemorySize));
    uintptr_t address     = reinterpret_cast<uintptr_t>(buffer);

    memset(buffer, 0, MemorySize);
    memcpy(buffer, EPOS, sizeof(EPOS));

    Console::cout << "\n *** EPOS is at core " << CPU::id() << " ***\n ";
    auto Entry = reinterpret_cast<void (*)(int)>(buffer);
    new VirtualCPU(address, MemorySize, Entry, 1);

    return 0;
}
