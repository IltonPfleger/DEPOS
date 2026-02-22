#include "Traits.hpp"
#include <Traits.hpp>

#include <abstractions/CPU.hpp>
#include <abstractions/VirtualCPU.hpp>
#include <drivers/virtio/Console.hpp>
#include <machine/Machine.hpp>
#include <network/NetworkAdapter.hpp>
#include <network/ethernet/ip/ARP.hpp>
#include <network/ethernet/ip/IPv4.hpp>
#include <network/ethernet/ip/TFTP.hpp>
#include <network/ethernet/ip/UDP.hpp>
#include <utils/Console.hpp>
#include <utils/string.hpp>

class LinuxDeviceTree {
    enum {
        FDT_BEGIN_NODE = 1,
        FDT_PROP = 3,
    };

  public:
    bool valid() { return CPU::htobe32(m_magic) == 0xD00DFEED; }

    bool edit(const char *node, const char *property, void *value, unsigned size) {
        unsigned structs_offset = CPU::htobe32(m_off_dt_struct);
        unsigned structs_size = CPU::htobe32(m_size_dt_struct);
        unsigned strings_offset = CPU::htobe32(m_off_dt_strings);
        unsigned char *structs = reinterpret_cast<unsigned char *>(this) + structs_offset;
        unsigned char *strings = reinterpret_cast<unsigned char *>(this) + strings_offset;
        unsigned char *current = structs;
        bool is_target_node = false;

        while (current < structs + structs_size) {
            unsigned token = CPU::htobe32(*(unsigned *)current);
            current += 4;

            if (token == FDT_BEGIN_NODE) {
                const char *name = (const char *)current;
                size_t len = strlen(name);

                if (strcmp(name, node) == 0) {
                    is_target_node = true;
                } else {
                    is_target_node = false;
                }

                current += (len + 1 + 3) & ~3;
            } else if (token == FDT_PROP) {
                unsigned len = CPU::htobe32(*(unsigned *)current);
                unsigned name_offset = CPU::htobe32(*(unsigned *)(current + 4));
                const char *name = (const char *)(strings + name_offset);
                current += 8;

                if (is_target_node && strcmp(name, property) == 0) {
                    if (len == size) {
                        memcpy(current, value, size);
                        return true;
                    }
                }
            }
        }
        return false;
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

unsigned char *align(unsigned char *p, long alignment) {
    long addr = reinterpret_cast<long>(p);
    addr = (addr + alignment - 1) & ~(alignment - 1);
    return reinterpret_cast<unsigned char *>(addr);
}

int main() {
    TraceIn();

    typedef Meta::GetFromTypeList<Traits<Ethernet>::Devices, 0>::Result Driver;
    typedef void (*Entry)(int, LinuxDeviceTree *);

    Driver::init();

    NetworkAdapter<Driver>::init();

    ARP<Driver>::init();

    constexpr long MB = 1024 * 1024;
    constexpr long LinuxMemorySize = 256 * MB;

    unsigned char *memory = reinterpret_cast<unsigned char *>(Memory::alloc(LinuxMemorySize));
    unsigned char *current = memory;

    TFTP<Driver> tftp("192.168.1.100");

    current = align(current, 2 * MB);
    unsigned char *kernel = current;
    size_t kernel_size = tftp.request("Image", kernel, LinuxMemorySize);
    current += kernel_size;

    current = align(current, MB);
    unsigned char *initramfs = current;
    size_t initramfs_size = tftp.request("initramfs.cpio", initramfs, LinuxMemorySize - kernel_size);
    current += initramfs_size;

    current = align(current, MB);
    LinuxDeviceTree *dtb = reinterpret_cast<LinuxDeviceTree *>(current);
    size_t dtb_size = tftp.request("guest.dtb", dtb, LinuxMemorySize - kernel_size - initramfs_size);
    current += dtb_size;

    if (!dtb->valid()) {
        Console::cout << "LinuxDeviceTree: Invalid!\n";
        return 1;
    }

    unsigned long memory_start_base = reinterpret_cast<long>(memory);
    unsigned memory_start_hi = static_cast<unsigned>(memory_start_base >> 32);
    unsigned memory_start_lo = static_cast<unsigned>(memory_start_base & 0xFFFFFFFF);
    unsigned memory_size_hi = static_cast<unsigned>(LinuxMemorySize >> 32);
    unsigned memory_size_lo = static_cast<unsigned>(LinuxMemorySize & 0xFFFFFFFF);
    unsigned memory_array[] = {CPU::htobe32(memory_start_hi), CPU::htobe32(memory_start_lo), CPU::htobe32(memory_size_hi),
                               CPU::htobe32(memory_size_lo)};
    unsigned initramfs_start = CPU::htobe32(static_cast<unsigned>(reinterpret_cast<long>(initramfs)));
    unsigned initramfs_end = CPU::htobe32(static_cast<unsigned>(reinterpret_cast<unsigned long>((initramfs + initramfs_size))));

    if (!dtb->edit("chosen", "linux,initrd-start", &initramfs_start, sizeof(initramfs_start))) {
        Console::cout << "LinuxDeviceTree: failed to update linux,initrd-start\n";
    };

    if (!dtb->edit("chosen", "linux,initrd-end", &initramfs_end, sizeof(initramfs_end))) {
        Console::cout << "LinuxDeviceTree: failed to update linux,initrd-end\n";
    }

    if (!dtb->edit("memory", "reg", &memory_array, sizeof(memory_array))) {
        Console::cout << "LinuxDeviceTree: failed to update memory\n";
    }

    auto entry = reinterpret_cast<Entry>(kernel);

    Console::cout << "\n *** Linux ***\n";

    new VirtualCPU(entry, MemoryMap::Entry{memory_start_base, memory_start_base + LinuxMemorySize}, 0, dtb);

    TraceOut();

    return 0;
}
