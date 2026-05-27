#include <Traits.hpp>
#include <architecture/CPU.hpp>
#include <hypervisor/DeviceTree.hpp>
#include <hypervisor/GenericVirtualMachine.hpp>
#include <hypervisor/virtio/Console.hpp>
#include <machine/Machine.hpp>
#include <utility/Console.hpp>

#define ___STR(X) #X
#define __STR(X) ___STR(X)

using namespace DEPOS;

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

unsigned char *align(unsigned char *pointer, long alignment) {
    uintptr_t address = reinterpret_cast<long>(pointer);
    address           = (address + alignment - 1) & ~(alignment - 1);
    return reinterpret_cast<unsigned char *>(address);
}

int main() {
    TraceIn();

    constexpr size_t MB              = 1024 * 1024;
    constexpr size_t LinuxMemorySize = 256 * MB;

    unsigned char *memory_start = reinterpret_cast<unsigned char *>(Memory::alloc(LinuxMemorySize));
    uintptr_t address           = reinterpret_cast<uintptr_t>(memory_start);
    unsigned char *current      = memory_start;

    current               = align(current, 2 * MB);
    unsigned char *kernel = current;
    current += sizeof(LinuxImage::Kernel);
    current         = align(current, MB);
    DeviceTree *dtb = reinterpret_cast<DeviceTree *>(current);
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

    Console::println("\n *** Linux is at core ", CPU::id(), " ***");

    typedef GenericVirtualMachine<Serial> MyMachine;
    MyMachine *vm = new MyMachine(kernel, LinuxMemorySize);
    vm->activate(0, dtb);

    return 0;
}
