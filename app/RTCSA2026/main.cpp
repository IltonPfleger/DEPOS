#include <Traits.hpp>
#include <architecture/CPU.hpp>
#include <architecture/VirtualCPU.hpp>
#include <hypervisor/DeviceTree.hpp>
#include <hypervisor/GenericVirtualMachine.hpp>
#include <hypervisor/VirtualSwitch.hpp>
#include <hypervisor/virtio/Console.hpp>
#include <hypervisor/virtio/Network.hpp>
#include <machine/Machine.hpp>
#include <network/link/LinkIPv4ToEthernet.hpp>
#include <network/protocols/TFTP.hpp>

using namespace DEPOS;

constexpr size_t MB                       = 1024 * 1024;
constexpr size_t VirtualMachineMemorySize = 256 * MB;

class EPOS_VirtualMachineLauncher {
  public:
    EPOS_VirtualMachineLauncher(TFTP *tftp)
        : tftp_(tftp),
          thread_(worker, this) {};

    static void *worker(void *pointer) {
        typedef Meta::GetFromTypeList<Traits<Ethernet>::Devices, 0>::Result NetworkDevice;
        typedef virtio::Network<VirtualSwitch<NetworkDevice>, 0x30200000> Network;
        typedef Meta::GetFromTypeList<Traits<UART>::Devices, 0>::Result SerialDevice;
        typedef virtio::Console<SerialDevice, 0x30000000> Serial;

        EPOS_VirtualMachineLauncher *self = reinterpret_cast<EPOS_VirtualMachineLauncher *>(pointer);
        auto *epos                        = static_cast<unsigned char *>(Memory::alloc(VirtualMachineMemorySize));
        self->tftp_->request(IPv4::Address(192, 168, 1, 100), "EPOS.bin", epos, VirtualMachineMemorySize);
        auto *vm = new GenericVirtualMachine<Serial, Network>(epos, VirtualMachineMemorySize);
        vm->activate(1, 15);
        return nullptr;
    }

  private:
    TFTP *tftp_;
    Thread thread_;
};

class Linux_VirtualMachineLauncher {
  public:
    Linux_VirtualMachineLauncher(TFTP *tftp)
        : tftp_(tftp),
          thread_(worker, this) {};

    static void *worker(void *pointer) {
        typedef Meta::GetFromTypeList<Traits<Ethernet>::Devices, 0>::Result NetworkDevice;
        typedef virtio::Network<NetworkDevice, 0x30200000> Network;
        typedef Meta::GetFromTypeList<Traits<UART>::Devices, 0>::Result SerialDevice;
        typedef virtio::Console<SerialDevice, 0x30000000> Serial;

        Linux_VirtualMachineLauncher *self = reinterpret_cast<Linux_VirtualMachineLauncher *>(pointer);

        auto *buffer = static_cast<unsigned char *>(Memory::alloc(VirtualMachineMemorySize));
        auto address = reinterpret_cast<uintptr_t>(buffer);

        auto *current    = buffer;
        size_t remaining = VirtualMachineMemorySize;

        auto *kernel       = current;
        size_t kernel_size = self->tftp_->request(IPv4::Address(192, 168, 1, 100), "Image", kernel, remaining);

        current += kernel_size;
        remaining -= kernel_size;

        DeviceTree *dtb = self->rdtb(&current, &remaining);
        // current = align(current, MB);

        // size_t dtb_size = self->tftp_->request(IPv4::Address(192, 168, 1, 100), "guest.dtb", dtb, remaining);

        // current += dtb_size;
        current = align(current, MB);
        remaining -= kernel_size;

        unsigned char *initramfs = current;
        size_t initramfs_size =
            self->tftp_->request(IPv4::Address(192, 168, 1, 100), "initramfs.cpio.gz", initramfs, remaining);

        current += initramfs_size;
        current = align(current, MB);
        remaining -= initramfs_size;

        ERROR(!dtb->valid(), "Invalid Device Tree!\n");

        unsigned int irq;
        unsigned int regs[4];

        // Memory
        regs[0] = CPU::htobe32(address >> 32);
        regs[1] = CPU::htobe32(address);
        regs[2] = CPU::htobe32(VirtualMachineMemorySize >> 32);
        regs[3] = CPU::htobe32(VirtualMachineMemorySize);
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

        // Network
        typedef Meta::GetFromTypeList<Traits<Ethernet>::Devices, 0>::Result NetworkDevice;
        typedef virtio::Network<NetworkDevice, 0x30200000> Network;
        irq     = CPU::htobe32(Network::IRQ);
        regs[0] = CPU::htobe32(Network::Address >> 32);
        regs[1] = CPU::htobe32(Network::Address);
        regs[2] = CPU::htobe32(0x0);
        regs[3] = CPU::htobe32(0x1000);
        dtb->edit("virtio_mmio@2", "compatible", "virtio,mmio", sizeof("virtio,mmio"));
        dtb->edit("virtio_mmio@2", "reg", regs, sizeof(regs));
        dtb->edit("virtio_mmio@2", "interrupts", &irq, sizeof(irq));

        Console::print("\n *** Linux ***\n");

        auto *vm = new GenericVirtualMachine<Serial, Network>(buffer, VirtualMachineMemorySize);
        vm->activate(0, dtb);

        return nullptr;
    }

  private:
    static unsigned char *align(unsigned char *pointer, long alignment) {
        uintptr_t address = reinterpret_cast<uintptr_t>(pointer);
        address           = (address + alignment - 1) & ~(alignment - 1);
        return reinterpret_cast<unsigned char *>(address);
    }

    DeviceTree *rdtb(unsigned char **current, size_t *remaining) {
        *current = align(*current, MB);

        auto *address = reinterpret_cast<DeviceTree *>(*current);
        size_t size   = tftp_->request(IPv4::Address(192, 168, 1, 100), "guest.dtb", address, *remaining);

        *current += size;
        *remaining -= size;

        return address;
    }

  private:
    TFTP *tftp_;
    Thread thread_;
    DeviceTree *dtb_;
};

class InterferenceLauncher {
  public:
    InterferenceLauncher() {
        for (auto &i : thread_)
            i = new Thread(worker);
    }

    static void *worker(void *) {
        while (1)
            ;
        return nullptr;
    }

  private:
    Thread *thread_[Traits<CPU>::Active];
};

int main() {
    typedef Meta::GetFromTypeList<Traits<Ethernet>::Devices, 0>::Result Device;

    auto *link = new LinkIPv4ToEthernet(*Device::instance());
    auto *ipv4 = new IPv4(IPv4::Address(192, 168, 1, 167), *link);
    auto *udp  = new UDP(ipv4);
    auto *tftp = new TFTP(*udp);

    // EPOS_VirtualMachineLauncher vm0(tftp);
    Linux_VirtualMachineLauncher vm1(tftp);
    InterferenceLauncher i0;

    while (1)
        Alarm::udelay(1'000'000);

    return 0;
}
