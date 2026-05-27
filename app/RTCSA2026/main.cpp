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
constexpr size_t VirtualMachineMemorySize = 128 * MB;

class ImageReceiver {
  public:
    ImageReceiver(TFTP *tftp)
        : tftp_(tftp),
          buffer_(Memory::alloc(VirtualMachineMemorySize), VirtualMachineMemorySize) {

        current_   = buffer_.data();
        remaining_ = buffer_.size();

        receive_linux();
        receive_dtb();
        receive_initramfs();
        receive_epos();
    }

    const Chunk &linux() const { return linux_; }
    const Chunk &dtb() const { return dtb_; }
    const Chunk &initramfs() const { return initramfs_; }
    const Chunk &epos() const { return epos_; }

  private:
    void receive_linux() {
        auto *start = current_;
        size_t size = tftp_->request(IPv4::Address(192, 168, 1, 100), "Image", start, remaining_);

        linux_ = Chunk(start, size);
        current_ += size;
        remaining_ -= size;
    }

    void receive_dtb() {
        current_ = align(current_, MB);

        auto *start = current_;
        size_t size = tftp_->request(IPv4::Address(192, 168, 1, 100), "guest.dtb", start, remaining_);

        dtb_ = Chunk(start, size);

        current_ += size;
        remaining_ -= size;
    }

    void receive_initramfs() {
        current_ = align(current_, MB);

        auto *start = current_;
        size_t size = tftp_->request(IPv4::Address(192, 168, 1, 100), "initramfs.cpio.gz", start, remaining_);

        initramfs_ = Chunk(start, size);

        current_ += size;
        remaining_ -= size;
    }

    void receive_epos() {
        current_ = align(current_, MB);

        auto *start = current_;
        size_t size = tftp_->request(IPv4::Address(192, 168, 1, 100), "EPOS.bin", start, remaining_);

        epos_ = Chunk(start, size);

        current_ += size;
        remaining_ -= size;
    }

    static unsigned char *align(unsigned char *p, long a) {
        uintptr_t addr = reinterpret_cast<uintptr_t>(p);
        addr           = (addr + a - 1) & ~(a - 1);
        return reinterpret_cast<unsigned char *>(addr);
    }

  private:
    TFTP *tftp_;
    Chunk buffer_;
    unsigned char *current_;
    size_t remaining_;

    Chunk linux_;
    Chunk dtb_;
    Chunk initramfs_;
    Chunk epos_;
};

class EPOS_VirtualMachineLauncher {
  public:
    EPOS_VirtualMachineLauncher(const Chunk &chunk,
                                Thread::Criterion criterion = Thread::Criterion(Thread::Criterion::NORMAL, 0))
        : chunk_(chunk),
          thread_(worker, this, criterion) {}

  private:
    static void *worker(void *pointer) {
        typedef Meta::GetFromTypeList<Traits<Ethernet>::Devices, 0>::Result NetworkDevice;
        typedef virtio::Network<VirtualSwitch<NetworkDevice>, 0x30200000> Network;
        typedef Meta::GetFromTypeList<Traits<UART>::Devices, 0>::Result SerialDevice;
        typedef virtio::Console<SerialDevice, 0x30000000> Serial;

        auto *self = reinterpret_cast<EPOS_VirtualMachineLauncher *>(pointer);

        auto *epos = static_cast<unsigned char *>(Memory::alloc(VirtualMachineMemorySize));

        memcpy(epos, reinterpret_cast<void *>(self->chunk_.start()), self->chunk_.size());

        auto *vm = new GenericVirtualMachine<Serial, Network>(epos, VirtualMachineMemorySize);

        vm->activate(1, 15);
        return nullptr;
    }

  private:
    const Chunk &chunk_;
    Thread thread_;
};

class Linux_VirtualMachineLauncher {
  public:
    Linux_VirtualMachineLauncher(const Chunk &kernel,
                                 const Chunk &initramfs,
                                 const Chunk &dtb,
                                 Thread::Criterion criterion = Thread::Criterion(Thread::Criterion::NORMAL, 0))
        : kernel_(kernel),
          initramfs_(initramfs),
          dtb_(dtb),
          thread_(worker, this, criterion) {}

  private:
    static void *worker(void *pointer) {
        typedef Meta::GetFromTypeList<Traits<UART>::Devices, 0>::Result SerialDevice;
        typedef virtio::Console<SerialDevice, 0x30000000> Serial;

        auto *self = reinterpret_cast<Linux_VirtualMachineLauncher *>(pointer);

        auto *buffer = static_cast<unsigned char *>(Memory::alloc(VirtualMachineMemorySize));

        unsigned char *current = buffer;
        uintptr_t base         = reinterpret_cast<uintptr_t>(buffer);

        memcpy(current, reinterpret_cast<void *>(self->kernel_.start()), self->kernel_.size());

        current += self->kernel_.size();
        current = align(current, MB);

        unsigned char *dtb_mem = current;

        memcpy(dtb_mem, reinterpret_cast<void *>(self->dtb_.start()), self->dtb_.size());

        DeviceTree *dtb = reinterpret_cast<DeviceTree *>(dtb_mem);

        ERROR(!dtb->valid(), "Invalid Device Tree!\n");

        current += self->dtb_.size();
        current = align(current, MB);

        unsigned char *initramfs = current;

        memcpy(initramfs, reinterpret_cast<void *>(self->initramfs_.start()), self->initramfs_.size());

        size_t initramfs_size = self->initramfs_.size();

        current += initramfs_size;
        current = align(current, MB);

        unsigned int regs[4];

        regs[0] = CPU::htobe32(base >> 32);
        regs[1] = CPU::htobe32(base);
        regs[2] = CPU::htobe32(VirtualMachineMemorySize >> 32);
        regs[3] = CPU::htobe32(VirtualMachineMemorySize);

        dtb->edit("memory@0", "reg", regs, sizeof(regs));

        regs[0] = CPU::htobe32(reinterpret_cast<uintptr_t>(initramfs) >> 32);
        regs[1] = CPU::htobe32(reinterpret_cast<uintptr_t>(initramfs));
        regs[2] = CPU::htobe32(reinterpret_cast<uintptr_t>(initramfs + initramfs_size) >> 32);
        regs[3] = CPU::htobe32(reinterpret_cast<uintptr_t>(initramfs + initramfs_size));

        dtb->edit("chosen", "linux,initrd-start", regs, sizeof(regs[0]) * 2);
        dtb->edit("chosen", "linux,initrd-end", &regs[2], sizeof(regs[0]) * 2);

        unsigned int irq = CPU::htobe32(Serial::IRQ);

        regs[0] = CPU::htobe32(Serial::Address >> 32);
        regs[1] = CPU::htobe32(Serial::Address);
        regs[2] = CPU::htobe32(0x0);
        regs[3] = CPU::htobe32(0x1000);

        dtb->edit("virtio_mmio@1", "compatible", "virtio,mmio", sizeof("virtio,mmio"));
        dtb->edit("virtio_mmio@1", "reg", regs, sizeof(regs));
        dtb->edit("virtio_mmio@1", "interrupts", &irq, sizeof(irq));

        Console::print("\n *** Linux ***\n");

        auto *vm = new GenericVirtualMachine<Serial>(buffer, VirtualMachineMemorySize);

        vm->activate(0, dtb);

        return nullptr;
    }

  private:
    static unsigned char *align(unsigned char *p, long a) {
        uintptr_t addr = reinterpret_cast<uintptr_t>(p);
        addr           = (addr + a - 1) & ~(a - 1);
        return reinterpret_cast<unsigned char *>(addr);
    }

  private:
    const Chunk &kernel_;
    const Chunk &initramfs_;
    const Chunk &dtb_;
    Thread thread_;
};

int main() {
    typedef Meta::GetFromTypeList<Traits<Ethernet>::Devices, 0>::Result Device;

    auto *link = new LinkIPv4ToEthernet(*Device::instance());
    auto *ipv4 = new IPv4(IPv4::Address(192, 168, 1, 167), *link);
    auto *udp  = new UDP(ipv4);
    auto *tftp = new TFTP(*udp);

    ImageReceiver imager(tftp);

    EPOS_VirtualMachineLauncher vm0(imager.epos(), Thread::Criterion(Thread::Criterion::NORMAL, 1));

    Linux_VirtualMachineLauncher vm1(imager.linux(), imager.initramfs(), imager.dtb(),
                                     Thread::Criterion(Thread::Criterion::NORMAL, 1));

    while (1)
        Alarm::udelay(1'000'000);

    return 0;
}
