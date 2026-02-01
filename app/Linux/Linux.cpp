#include "Traits.hpp"
#include <Traits.hpp>
#include <machine/Machine.hpp>
#include <utils/Console.hpp>
#include <utils/string.hpp>

class DTB {
    enum {
        FDT_BEGIN_NODE = 1,
        FDT_PROP = 3,
    };

  public:
    bool valid() { return CPU::htobe32(m_magic) == 0xD00DFEED; }

    bool edit(const char *property, void *value, unsigned int size) {
        unsigned int structs_offset = CPU::htobe32(m_off_dt_struct);
        unsigned int structs_size = CPU::htobe32(m_size_dt_struct);
        unsigned int strings_offset = CPU::htobe32(m_off_dt_strings);
        unsigned char *structs = reinterpret_cast<unsigned char *>(this) + structs_offset;
        unsigned char *strings = reinterpret_cast<unsigned char *>(this) + strings_offset;
        unsigned char *current = structs;

        while (current < structs + structs_size) {
            unsigned int token = CPU::htobe32(*(unsigned int *)current);
            current += 4;

            if (token == FDT_BEGIN_NODE) {
                const char *name = (const char *)current;
                size_t len = strlen(name);
                current += (len + 1 + 3) & ~3;
            } else if (token == FDT_PROP) {
                unsigned int len = CPU::htobe32(*(unsigned int *)current);
                unsigned int name_offset = CPU::htobe32(*(unsigned int *)(current + 4));
                current += 8;

                const char *name = (const char *)(strings + name_offset);
                if (strcmp(name, property) == 0) {

                    if (len == 4 && size == 4) {
                        uint32_t v = CPU::htobe32(*(uint32_t *)value);
                        memcpy(current, &v, 4);
                        return true;
                    }
                }
            }
        }
        return false;
    }

  private:
    unsigned int m_magic;
    unsigned int m_totalsize;
    unsigned int m_off_dt_struct;
    unsigned int m_off_dt_strings;
    unsigned int m_off_mem_rsvmap;
    unsigned int m_version;
    unsigned int m_last_comp_version;
    unsigned int m_boot_cpuid_phys;
    unsigned int m_size_dt_strings;
    unsigned int m_size_dt_struct;
};

alignas(2 * 1024 * 1024) volatile unsigned char g_image[] = {
#include __STR(__KERNEL__)
};

alignas(1 * 1024 * 1024) volatile unsigned char g_dtb[] = {
#include __STR(__DTB__)
};

alignas(1 * 1024 * 1024) volatile unsigned char g_initramfs[] = {
#include __STR(__INITRAMFS__)
};

int main() {
    Console::println("Linux:     %p\n", g_image);
    Console::println("DTB:       %p\n", g_dtb);
    Console::println("Initramfs: %p %d\n", g_initramfs, sizeof(g_initramfs));

    typedef void (*Entry)(unsigned int, DTB *);
    auto entry = reinterpret_cast<Entry>(g_image);
    DTB *dtb = reinterpret_cast<DTB *>(const_cast<unsigned char *>(g_dtb));

    if (!dtb->valid()) {
        Console::print("DTB: Invalid!\n");
        return 1;
    }

    unsigned int initramfs_start = static_cast<unsigned int>(reinterpret_cast<unsigned long>(g_initramfs));
    unsigned int initramfs_end = static_cast<unsigned int>(reinterpret_cast<unsigned long>(g_initramfs) + sizeof(g_initramfs));

    if (!dtb->edit("linux,initrd-start", &initramfs_start, sizeof(initramfs_start))) {
        Console::print("DTB: failed to update linux,initrd-start\n");
    };

    if (!dtb->edit("linux,initrd-end", &initramfs_end, sizeof(initramfs_end))) {
        Console::print("DTB: failed to update linux,initrd-end\n");
    }

    entry(CPU::id(), dtb);
    return 0;
}
