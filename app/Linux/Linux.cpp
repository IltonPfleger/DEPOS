#include <Traits.hpp>
#include <architecture/CPU.hpp>
// #include <drivers/ethernet/Dummy.hpp>
#include <hypervisor/DeviceTreeBlob.hpp>
#include <hypervisor/GenericVirtualMachine.hpp>
#include <hypervisor/virtio/Console.hpp>
#include <machine/Machine.hpp>
#include <macros.hpp>
#include <shared/console/Console.hpp>
#include <shared/libraries/libc/string.h>

using namespace DEPOS;

struct LinuxImageObjects {
    static constexpr unsigned char Kernel[] = {
#include __STRINGIFY(__KERNEL__)
    };

    static constexpr unsigned char DeviceTree[] = {
#include __STRINGIFY(__DTB__)
    };

    static constexpr unsigned char Initramfs[] = {
#include __STRINGIFY(__INITRAMFS__)
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

    unsigned char *memory_start = reinterpret_cast<unsigned char *>(Memory::alloc(LinuxMemorySize));
    uintptr_t address           = reinterpret_cast<uintptr_t>(memory_start);
    unsigned char *current      = memory_start;

    current               = align(current, 2 * MB);
    unsigned char *kernel = current;
    current += sizeof(LinuxImageObjects::Kernel);
    current   = align(current, MB);
    auto *dtb = reinterpret_cast<DeviceTreeBlob *>(current);
    current += sizeof(LinuxImageObjects::DeviceTree);
    current = align(current, MB);

    unsigned char *initramfs = current;
    size_t initramfs_size    = sizeof(LinuxImageObjects::Initramfs);
    current += initramfs_size;
    current = align(current, MB);

    memcpy(kernel, LinuxImageObjects::Kernel, sizeof(LinuxImageObjects::Kernel));
    memcpy(dtb, LinuxImageObjects::DeviceTree, sizeof(LinuxImageObjects::DeviceTree));
    memcpy(initramfs, LinuxImageObjects::Initramfs, sizeof(LinuxImageObjects::Initramfs));

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
    typedef meta::GetFromTypeList<Traits<UART>::Devices, 0>::Result SerialDevice;
    typedef virtio::Console<SerialDevice, 0x30000000> Serial;
    irq     = CPU::htobe32(Serial::IRQ);
    regs[0] = CPU::htobe32(Serial::Address >> 32);
    regs[1] = CPU::htobe32(Serial::Address);
    regs[2] = CPU::htobe32(0x0);
    regs[3] = CPU::htobe32(0x1000);
    dtb->edit("virtio_mmio@1", "compatible", "virtio,mmio", sizeof("virtio,mmio"));
    dtb->edit("virtio_mmio@1", "reg", regs, sizeof(regs));
    dtb->edit("virtio_mmio@1", "interrupts", &irq, sizeof(irq));

    // Network
    // typedef virtio::Network<Dummy, 0x30200000> Network;
    // irq     = CPU::htobe32(Network::IRQ);
    // regs[0] = CPU::htobe32(Network::Address >> 32);
    // regs[1] = CPU::htobe32(Network::Address);
    // regs[2] = CPU::htobe32(0x0);
    // regs[3] = CPU::htobe32(0x1000);
    // dtb->edit("virtio_mmio@2", "compatible", "virtio,mmio", sizeof("virtio,mmio"));
    // dtb->edit("virtio_mmio@2", "reg", regs, sizeof(regs));
    // dtb->edit("virtio_mmio@2", "interrupts", &irq, sizeof(irq));

    Console::print("\n *** Linux is at core ", CPU::id(), " ***\n ");
    CPU::mb();

    typedef GenericVirtualMachine<Serial> MyMachine;
    MyMachine *vm = new MyMachine(kernel, LinuxMemorySize);
    vm->start(0, dtb);

    return 0;
}
