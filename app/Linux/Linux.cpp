#include "Traits.hpp"
#include <Traits.hpp>
#include <machine/Machine.hpp>
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
    addr = (addr + alignment - 1) & ~(alignment - 1);
    return reinterpret_cast<unsigned char *>(addr);
}

int main() {
    constexpr long MB = 1024 * 1024;
    constexpr long LinuxMemorySize = 128 * 1024 * 1024;
    typedef void (*Entry)(unsigned, LinuxDeviceTree *);

    unsigned char *memory_start = reinterpret_cast<unsigned char *>(Memory::alloc(LinuxMemorySize));
    unsigned char *current = memory_start;

    current = align(current, 2 * MB);
    unsigned char *kernel = current;
    current += sizeof(LinuxImage::Kernel);
    current = align(current, MB);
    LinuxDeviceTree *dtb = reinterpret_cast<LinuxDeviceTree *>(current);
    current += sizeof(LinuxImage::Dtb);
    current = align(current, MB);
    unsigned char *initramfs = current;
    current += sizeof(LinuxImage::Initramfs);
    current = align(current, MB);

    memcpy(kernel, LinuxImage::Kernel, sizeof(LinuxImage::Kernel));
    memcpy(dtb, LinuxImage::Dtb, sizeof(LinuxImage::Dtb));
    memcpy(initramfs, LinuxImage::Initramfs, sizeof(LinuxImage::Initramfs));

    if (!dtb->valid()) {
        Console::print("LinuxDeviceTree: Invalid!\n");
        return 1;
    }

    long memory_start_base = reinterpret_cast<long>(memory_start);
    unsigned memory_start_hi = static_cast<unsigned>(memory_start_base >> 32);
    unsigned memory_start_lo = static_cast<unsigned>(memory_start_base & 0xFFFFFFFF);
    unsigned memory_size_hi = static_cast<unsigned>(LinuxMemorySize >> 32);
    unsigned memory_size_lo = static_cast<unsigned>(LinuxMemorySize & 0xFFFFFFFF);
    unsigned memory[] = {CPU::htobe32(memory_start_hi), CPU::htobe32(memory_start_lo), CPU::htobe32(memory_size_hi),
                         CPU::htobe32(memory_size_lo)};
    unsigned initramfs_start = CPU::htobe32(static_cast<unsigned>(reinterpret_cast<long>(initramfs)));
    unsigned initramfs_end =
        CPU::htobe32(static_cast<unsigned>(reinterpret_cast<long>(initramfs) + sizeof(LinuxImage::Initramfs)));

    if (!dtb->edit("chosen", "linux,initrd-start", &initramfs_start, sizeof(initramfs_start))) {
        Console::print("LinuxDeviceTree: failed to update linux,initrd-start\n");
    };

    if (!dtb->edit("chosen", "linux,initrd-end", &initramfs_end, sizeof(initramfs_end))) {
        Console::print("LinuxDeviceTree: failed to update linux,initrd-end\n");
    }

    if (!dtb->edit("memory", "reg", &memory, sizeof(memory))) {
        Console::print("LinuxDeviceTree: failed to update memory\n");
    }

    auto entry = reinterpret_cast<Entry>(kernel);

    entry(CPU::id(), dtb);
    return 0;
}
