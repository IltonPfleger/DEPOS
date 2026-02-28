#include <Traits.hpp>
#include <abstractions/Cache.hpp>
#include <abstractions/VirtualCPU.hpp>
#include <architecture/CPU.hpp>
#include <drivers/virtio/Console.hpp>
#include <machine/Machine.hpp>
#include <network/NIC.hpp>
#include <network/ethernet/ip/ARP.hpp>
#include <network/ethernet/ip/IPv4.hpp>
#include <network/ethernet/ip/TFTP.hpp>
#include <network/ethernet/ip/UDP.hpp>
#include <utils/Console.hpp>
#include <utils/string.hpp>

using namespace DEPOS;

class LinuxDeviceTree {
    enum { FDT_BEGIN_NODE = 1, FDT_END_NODE = 2, FDT_PROP = 3, FDT_NOP = 4, FDT_END = 9 };

  public:
    bool valid() { return CPU::be32toh(m_magic) == 0xD00DFEED; }

    bool edit(const char *node, const char *property, void *value, unsigned size) {
        unsigned structs_offset = CPU::be32toh(m_off_dt_struct);
        unsigned structs_size   = CPU::be32toh(m_size_dt_struct);
        unsigned strings_offset = CPU::be32toh(m_off_dt_strings);

        unsigned char *structs = reinterpret_cast<unsigned char *>(this) + structs_offset;
        unsigned char *strings = reinterpret_cast<unsigned char *>(this) + strings_offset;
        unsigned char *current = structs;
        bool is_target_node    = false;

        while (current < structs + structs_size) {
            unsigned token = CPU::be32toh(*(unsigned *)current);
            current += 4;

            if (token == FDT_BEGIN_NODE) {
                const char *name = (const char *)current;
                // Usando strcmp simples como solicitado
                if (strcmp(name, node) == 0) {
                    is_target_node = true;
                } else {
                    is_target_node = false;
                }
                current += (strlen(name) + 1 + 3) & ~3;
            } else if (token == FDT_PROP) {
                unsigned prop_len     = CPU::be32toh(*(unsigned *)current);
                unsigned name_offset  = CPU::be32toh(*(unsigned *)(current + 4));
                const char *prop_name = (const char *)(strings + name_offset);
                current += 8;

                if (is_target_node && strcmp(prop_name, property) == 0) {
                    if (prop_len == size) {
                        memcpy(current, value, size);
                        // Não damos return true aqui imediatamente para manter
                        // o ponteiro 'current' íntegro, ou garantimos que ele
                        // pule o valor antes de retornar.
                        current += (prop_len + 3) & ~3;
                        return true;
                    }
                }

                // --- O ERRO ESTAVA AQUI ---
                // Você precisa pular o conteúdo da propriedade (alinhado a 4
                // bytes) mesmo que não seja a propriedade que você quer editar.
                current += (prop_len + 3) & ~3;
            } else if (token == FDT_END_NODE) {
                is_target_node = false;
            } else if (token == FDT_END) {
                break;
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
    addr      = (addr + alignment - 1) & ~(alignment - 1);
    return reinterpret_cast<unsigned char *>(addr);
}

int main() {

    typedef Meta::GetFromTypeList<Traits<Ethernet>::Devices, 0>::Result Driver;
    typedef void (*Entry)(int, LinuxDeviceTree *);

    Driver::init();

    NIC<Driver>::init();

    constexpr long MB              = 1024 * 1024;
    constexpr long LinuxMemorySize = 256 * MB;

    unsigned char *buffer = reinterpret_cast<unsigned char *>(Memory::alloc(LinuxMemorySize));
    memset(buffer, 0, LinuxMemorySize);

    unsigned char *current = buffer;

    TFTP<Driver> tftp("192.168.1.100");

    unsigned char *kernel = current;
    size_t kernel_size    = tftp.request("Image", kernel, LinuxMemorySize);
    current += kernel_size;

    current              = align(current, MB);
    LinuxDeviceTree *dtb = reinterpret_cast<LinuxDeviceTree *>(current);
    size_t dtb_size      = tftp.request("guest.dtb", dtb, LinuxMemorySize - kernel_size);
    current += dtb_size;

    current                  = align(current, MB);
    unsigned char *initramfs = current;
    size_t initramfs_size =
        tftp.request("initramfs.cpio", initramfs, LinuxMemorySize - kernel_size - dtb_size);

    if (!dtb->valid()) {
        Console::cout << "LinuxDeviceTree: Invalid!\n";
        return 1;
    }

    unsigned long memory_start_base = reinterpret_cast<long>(buffer);
    unsigned memory_start_hi        = static_cast<unsigned>(memory_start_base >> 32);
    unsigned memory_start_lo        = static_cast<unsigned>(memory_start_base & 0xFFFFFFFF);
    unsigned memory_size_hi         = static_cast<unsigned>(LinuxMemorySize >> 32);
    unsigned memory_size_lo         = static_cast<unsigned>(LinuxMemorySize & 0xFFFFFFFF);
    unsigned memory[]               = {CPU::htobe32(memory_start_hi), CPU::htobe32(memory_start_lo),
                                       CPU::htobe32(memory_size_hi), CPU::htobe32(memory_size_lo)};

    long initramfs_start = CPU::htobe64(reinterpret_cast<long>(initramfs));
    long initramfs_end   = CPU::htobe64(reinterpret_cast<long>(initramfs) + initramfs_size);

    if (!dtb->edit("chosen", "linux,initrd-start", &initramfs_start, sizeof(initramfs_start))) {
        Console::cout << "LinuxDeviceTree: failed to update linux,initrd-start\n";
    };

    if (!dtb->edit("chosen", "linux,initrd-end", &initramfs_end, sizeof(initramfs_end))) {
        Console::cout << "LinuxDeviceTree: failed to update linux,initrd-end\n";
    }

    if (!dtb->edit("memory", "reg", &memory, sizeof(memory))) {
        Console::cout << "LinuxDeviceTree: failed to update memory\n";
    }

    Entry entry = reinterpret_cast<Entry>(kernel);

    Console::cout << "\n *** Linux ***\n";

    new VirtualCPU(entry, MemoryMap::Entry{memory_start_base, memory_start_base + LinuxMemorySize},
                   0, dtb);

    TraceOut();

    return 0;
}
