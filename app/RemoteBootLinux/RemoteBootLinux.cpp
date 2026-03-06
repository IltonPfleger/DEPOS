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

    void edit(const char *node, const char *property, const void *value, unsigned size) {
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
                        current += (prop_len + 3) & ~3;
                        return;
                    }
                }

                current += (prop_len + 3) & ~3;
            } else if (token == FDT_END_NODE) {
                is_target_node = false;
            } else if (token == FDT_END) {
                break;
            }
        }
        ERROR(true, node, property, const_cast<void *>(value), size);
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
    typedef Meta::GetFromTypeList<Traits<Ethernet>::Devices, 0>::Result Device;

    constexpr long MB              = 1024 * 1024;
    constexpr long LinuxMemorySize = 256 * MB;

    unsigned char *buffer = reinterpret_cast<unsigned char *>(Memory::alloc(LinuxMemorySize));
    uintptr_t address     = reinterpret_cast<uintptr_t>(buffer);

    memset(buffer, 0, LinuxMemorySize);

    unsigned char *current = buffer;
    unsigned int remaining = LinuxMemorySize;

    TFTP<Device> tftp("192.168.1.100");

    unsigned char *kernel = current;
    size_t kernel_size    = tftp.request("Image", kernel, remaining);
    current += kernel_size;
    remaining -= kernel_size;

    current              = align(current, MB);
    LinuxDeviceTree *dtb = reinterpret_cast<LinuxDeviceTree *>(current);
    size_t dtb_size      = tftp.request("guest.dtb", dtb, remaining);
    current += dtb_size;
    remaining -= kernel_size;

    current                  = align(current, MB);
    unsigned char *initramfs = current;
    size_t initramfs_size    = tftp.request("initramfs.cpio", initramfs, remaining);
    remaining -= initramfs_size;

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
    typedef Meta::GetFromTypeList<Traits<Virtual>::Devices, 0>::Result Serial;
    irq     = CPU::htobe32(Serial::IRQ);
    regs[0] = CPU::htobe32(Serial::Address >> 32);
    regs[1] = CPU::htobe32(Serial::Address);
    regs[2] = CPU::htobe32(0x0);
    regs[3] = CPU::htobe32(0x1000);
    dtb->edit("virtio_mmio@1", "compatible", "virtio,mmio", sizeof("virtio,mmio"));
    dtb->edit("virtio_mmio@1", "reg", regs, sizeof(regs));
    dtb->edit("virtio_mmio@1", "interrupts", &irq, sizeof(irq));

    // Network
    typedef Meta::GetFromTypeList<Traits<Virtual>::Devices, 1>::Result Network;
    irq     = CPU::htobe32(Network::IRQ);
    regs[0] = CPU::htobe32(Network::Address >> 32);
    regs[1] = CPU::htobe32(Network::Address);
    regs[2] = CPU::htobe32(0x0);
    regs[3] = CPU::htobe32(0x1000);
    dtb->edit("virtio_mmio@2", "compatible", "virtio,mmio", sizeof("virtio,mmio"));
    dtb->edit("virtio_mmio@2", "reg", regs, sizeof(regs));
    dtb->edit("virtio_mmio@2", "interrupts", &irq, sizeof(irq));

    Console::cout << "\n *** Linux is at core " << CPU::id() << " ***\n";

    auto Linux = reinterpret_cast<void (*)(int, LinuxDeviceTree *)>(kernel);
    new VirtualCPU(address, LinuxMemorySize, Linux, 0, dtb);
    return 0;
}
