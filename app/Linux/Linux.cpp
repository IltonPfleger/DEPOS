#include <Traits.hpp>
#include <architecture/CPU.hpp>
#include <drivers/hypervisor/GenericVirtualMachine.hpp>
#include <drivers/virtio/Console.hpp>
#include <machine/Machine.hpp>
#include <utils/Console.hpp>
#include <utils/string.hpp>

using namespace DEPOS;

class LinuxDeviceTree {
    enum {
        FDT_BEGIN_NODE = 1,
        FDT_PROP       = 3,
    };

  public:
    bool valid() { return CPU::htobe32(m_magic) == 0xD00DFEED; }

    void edit(const char *node, const char *property, const void *value, unsigned size) {
        unsigned structs_offset = CPU::htobe32(m_off_dt_struct);
        unsigned structs_size   = CPU::htobe32(m_size_dt_struct);
        unsigned strings_offset = CPU::htobe32(m_off_dt_strings);
        unsigned char *structs  = reinterpret_cast<unsigned char *>(this) + structs_offset;
        unsigned char *strings  = reinterpret_cast<unsigned char *>(this) + strings_offset;
        unsigned char *current  = structs;
        bool is_target_node     = false;

        while (current < structs + structs_size) {
            unsigned token = CPU::htobe32(*(unsigned *)current);
            current += 4;

            if (token == FDT_BEGIN_NODE) {
                const char *name = (const char *)current;
                size_t len       = strlen(name);

                if (strcmp(name, node) == 0) {
                    is_target_node = true;
                } else {
                    is_target_node = false;
                }

                current += (len + 1 + 3) & ~3;
            } else if (token == FDT_PROP) {
                unsigned len         = CPU::htobe32(*(unsigned *)current);
                unsigned name_offset = CPU::htobe32(*(unsigned *)(current + 4));
                const char *name     = (const char *)(strings + name_offset);
                current += 8;

                if (is_target_node && strcmp(name, property) == 0) {
                    if (len == size) {
                        memcpy(current, value, size);
                        return;
                    }
                }
            }
        }
        ERROR(true, node, property);
    }

  private:
    unsigned m_magic;
    unsigned m_totalsize;
    unsigned m_off_dt_struct;
    unsigned m_off_dt_strings;
    unsigned m_off_mem_rsvmap;
    unsigned m_version;
    unsigned m_last_comp_version;
    unsigned m_boot_cpuid_phys;
    unsigned m_size_dt_strings;
    unsigned m_size_dt_struct;
};

#define ___STR(X) #X
#define __STR(X) ___STR(X)

struct LinuxImage {
    alignas(2 * 1024 * 1024) static constexpr unsigned char Kernel[] = {
#include __STR(__KERNEL__)
    };

    alignas(1 * 1024 * 1024) static constexpr unsigned char Dtb[] = {
#include __STR(__DTB__)
    };

    alignas(1 * 1024 * 1024) static constexpr unsigned char Initramfs[] = {
#include __STR(__INITRAMFS__)
    };
};

unsigned char *align(unsigned char *p, long alignment) {
    long addr = reinterpret_cast<long>(p);
    addr      = (addr + alignment - 1) & ~(alignment - 1);
    return reinterpret_cast<unsigned char *>(addr);
}

int main() {
    TraceIn();

    constexpr long MB              = 1024 * 1024;
    constexpr long LinuxMemorySize = 256 * 1024 * 1024;
    typedef void (*Entry)(int, LinuxDeviceTree *);

    unsigned char *memory_start = reinterpret_cast<unsigned char *>(Memory::alloc(LinuxMemorySize));
    uintptr_t address           = reinterpret_cast<uintptr_t>(memory_start);
    unsigned char *current      = memory_start;

    current               = align(current, 2 * MB);
    unsigned char *kernel = current;
    current += sizeof(LinuxImage::Kernel);
    current              = align(current, MB);
    LinuxDeviceTree *dtb = reinterpret_cast<LinuxDeviceTree *>(current);
    current += sizeof(LinuxImage::Dtb);
    current = align(current, MB);

    unsigned char *initramfs = current;
    size_t initramfs_size    = sizeof(LinuxImage::Initramfs);
    current += initramfs_size;
    current = align(current, MB);

    memcpy(kernel, LinuxImage::Kernel, sizeof(LinuxImage::Kernel));
    memcpy(dtb, LinuxImage::Dtb, sizeof(LinuxImage::Dtb));
    memcpy(initramfs, LinuxImage::Initramfs, sizeof(LinuxImage::Initramfs));

    ERROR(!dtb->valid(), "Invalid Device Tree!\n");

    unsigned int irq;
    unsigned int regs[4];

    // Memory
    regs[0] = CPU::htobe32(address >> 32);
    regs[1] = CPU::htobe32(address);
    regs[2] = CPU::htobe32(LinuxMemorySize >> 32);
    regs[3] = CPU::htobe32(LinuxMemorySize);
    dtb->edit("memory@0", "reg", regs, sizeof(regs));

    // Initramfs
    regs[0] = CPU::htobe32(reinterpret_cast<unsigned long>(initramfs) >> 32);
    regs[1] = CPU::htobe32(reinterpret_cast<unsigned long>(initramfs));
    regs[2] = CPU::htobe32(reinterpret_cast<unsigned long>(initramfs + initramfs_size) >> 32);
    regs[3] = CPU::htobe32(reinterpret_cast<unsigned long>(initramfs + initramfs_size));
    dtb->edit("chosen", "linux,initrd-start", regs, sizeof(regs[0]) * 2);
    dtb->edit("chosen", "linux,initrd-end", &regs[2], sizeof(regs[0]) * 2);

    // Serial
    typedef Meta::GetFromTypeList<Traits<UART>::Devices, 0>::Result SerialDevice;
    typedef virtio::Console<SerialDevice, 0x30000000> Serial;
    irq     = CPU::htobe32(Serial::IRQ);
    regs[0] = CPU::htobe32(Serial::Address >> 32);
    regs[1] = CPU::htobe32(Serial::Address);
    regs[2] = CPU::htobe32(0x0);
    regs[3] = CPU::htobe32(0x1000);
    dtb->edit("virtio_mmio@1", "compatible", "virtio,mmio", sizeof("virtio,mmio"));
    dtb->edit("virtio_mmio@1", "reg", regs, sizeof(regs));
    dtb->edit("virtio_mmio@1", "interrupts", &irq, sizeof(irq));

    auto entry = reinterpret_cast<Entry>(kernel);

    Console::cout << "\n *** Linux is at core " << CPU::id() << " ***\n ";
    CPU::mb();

    new GenericVirtualMachine<Serial>(entry, address, LinuxMemorySize, 0, dtb);

    return 0;
}
