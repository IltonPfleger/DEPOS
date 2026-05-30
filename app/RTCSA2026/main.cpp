#include <Traits.hpp>
#include <architecture/CPU.hpp>
#include <architecture/VirtualCPU.hpp>
#include <hypervisor/GenericVirtualMachine.hpp>
#include <hypervisor/VirtualSwitch.hpp>
#include <hypervisor/dtb/FDT_Builder.hpp>
#include <hypervisor/virtio/Console.hpp>
#include <hypervisor/virtio/Network.hpp>
#include <machine/Machine.hpp>
#include <network/link/LinkIPv4ToEthernet.hpp>
#include <network/protocols/TFTP.hpp>
#include <utility/Span.hpp>

using namespace DEPOS;

constexpr size_t MB                       = 1024 * 1024;
constexpr size_t VirtualMachineMemorySize = 128 * MB;

class Receiver {
  public:
    Receiver(TFTP *tftp)
        : tftp_(tftp),
          buffer_(static_cast<uint8_t *>(Memory::alloc(VirtualMachineMemorySize)), VirtualMachineMemorySize) {

        current_   = buffer_.data();
        remaining_ = buffer_.length();

        rlinux();
        rinitramfs();
        repos();
    }

    const auto &linux() const { return linux_; }
    const auto &initramfs() const { return initramfs_; }
    const auto &epos() const { return epos_; }

  private:
    void rlinux() {
        size_t size          = tftp_->request(IPv4::Address(192, 168, 1, 100), "Image", current_, remaining_);
        const uint8_t *start = current_;
        linux_               = Span(start, size);
        current_ += size;
        remaining_ -= size;
    }

    void rinitramfs() {
        size_t size          = tftp_->request(IPv4::Address(192, 168, 1, 100), "initramfs.cpio", current_, remaining_);
        const uint8_t *start = current_;
        initramfs_           = Span(start, size);
        current_ += size;
        remaining_ -= size;
    }

    void repos() {
        size_t size          = tftp_->request(IPv4::Address(192, 168, 1, 100), "EPOS.bin", current_, remaining_);
        const uint8_t *start = current_;
        epos_                = Span(start, size);
        current_ += size;
        remaining_ -= size;
    }

  private:
    TFTP *tftp_;
    Span<uint8_t> buffer_;
    unsigned char *current_;
    size_t remaining_;
    Span<const uint8_t> linux_;
    Span<const uint8_t> initramfs_;
    Span<const uint8_t> epos_;
};

class LinuxLauncher {
  public:
    using SerialDevice = Meta::GetFromTypeList<Traits<UART>::Devices, 0>::Result;
    using Serial       = virtio::Console<SerialDevice, 0x30000000>;
    using LinuxMachine = GenericVirtualMachine<Serial>;

    LinuxLauncher(size_t size, Span<const uint8_t> kernel, Span<const uint8_t> initramfs, Thread::Criterion criterion)
        : size_(size),
          start_(nullptr),
          initramfs_(initramfs),
          dtb_(nullptr),
          vm_(nullptr) {
        start_ = static_cast<unsigned char *>(Memory::alloc(size_));

        uint8_t *current = align(start_, 4 * MB);
        memcpy(current, kernel, kernel.length());
        current += kernel.length();

        current = align(current, 4 * MB);
        memcpy(current, initramfs, initramfs.length());
        initramfs_ = Span<const uint8_t>(current, initramfs.length());
        current += initramfs.length();

        current = align(current, 4 * MB);
        dtb(current, size_ - initramfs.length() - kernel.length());

        new Thread(worker, this, criterion);
    }

    static void *worker(void *pointer) {
        LinuxLauncher *self = reinterpret_cast<LinuxLauncher *>(pointer);
        Console::println("\n *** Linux is at core ", CPU::id(), " ***");
        LinuxMachine *vm = new LinuxMachine(self->start_, self->size_);
        vm->activate(0, self->dtb_);
        return nullptr;
    }

    static unsigned char *align(unsigned char *pointer, long alignment) {
        uintptr_t address = reinterpret_cast<long>(pointer);
        address           = (address + alignment - 1) & ~(alignment - 1);
        return reinterpret_cast<unsigned char *>(address);
    }

    size_t dtb(void *buffer, size_t capacity) {
        FDT_Builder fdt(buffer, capacity);

        dtb_                  = static_cast<unsigned char *>(buffer);
        uint64_t base         = reinterpret_cast<uint64_t>(start_);
        uint64_t initrd_start = reinterpret_cast<uint64_t>(initramfs_.data());
        uint64_t initrd_end   = initrd_start + initramfs_.length();

        fdt.begin("");
        {
            fdt.add("#address-cells", 2);
            fdt.add("#size-cells", 2);
            fdt.add("compatible", "riscv-virtio");
            fdt.add("model", "riscv-virtio,qemu");

            fdt.begin("chosen");
            {
                fdt.add("bootargs", "console=hvc0 loglevel=8");
                uint32_t regs0[] = {CPU::hi32(initrd_start), CPU::lo32(initrd_start)};
                fdt.add("linux,initrd-start", regs0, 2);
                uint32_t regs1[] = {CPU::hi32(initrd_end), CPU::lo32(initrd_end)};
                fdt.add("linux,initrd-end", regs1, 2);
            }
            fdt.end();

            fdt.begin("cpus");
            {
                fdt.add("#address-cells", 1);
                fdt.add("#size-cells", 0u);
                fdt.add("timebase-frequency", 0x989680);
                fdt.begin("cpu@0");
                {
                    fdt.add("device_type", "cpu");
                    fdt.add("reg", 0u);
                    fdt.add("status", "okay");
                    fdt.add("compatible", "riscv");
                    fdt.add("riscv,isa", "rv64imafdcsu");
                    fdt.add("mmu-type", "riscv,sv39");
                    fdt.begin("interrupt-controller");
                    {
                        fdt.add("#interrupt-cells", 1);
                        fdt.add("interrupt-controller");
                        fdt.add("compatible", "riscv,cpu-intc");
                        fdt.add("phandle", 0x01);
                    }
                    fdt.end();
                }
                fdt.end();
            }
            fdt.end();

            fdt.begin("memory");
            {
                fdt.add("device_type", "memory");
                uint32_t regs[] = {CPU::hi32(base), CPU::lo32(base), CPU::hi32(size_), CPU::lo32(size_)};
                fdt.add("reg", regs, 4);
            }
            fdt.end();

            fdt.begin("soc");
            {
                fdt.add("#address-cells", 2);
                fdt.add("#size-cells", 2);
                fdt.add("compatible", "simple-bus");
                fdt.add("ranges");

                fdt.begin("interrupt-controller@c000000");
                {
                    fdt.add("compatible", "riscv,plic0");

                    uint32_t regs0[] = {0x00, 0xc000000, 0x00, 0x4000000};
                    fdt.add("reg", regs0, 4);

                    fdt.add("interrupt-controller");
                    fdt.add("#interrupt-cells", 1);
                    fdt.add("riscv,ndev", 0x35);

                    uint32_t regs1[] = {0x01, 0x0b, 0x01, 0x09};
                    fdt.add("interrupts-extended", regs1, 4);
                    fdt.add("phandle", 0x02);
                }
                fdt.end();

                fdt.begin("virtio_mmio@30000000");
                {
                    uint64_t address = 0x30000000;
                    uint32_t irq     = 32;
                    uint32_t regs[]  = {CPU::hi32(address), CPU::lo32(address), 0x00, 0x1000};
                    fdt.add("compatible", "virtio,mmio");
                    fdt.add("reg", regs, 4);
                    fdt.add("interrupts", irq);
                    fdt.add("interrupt-parent", 0x02);
                }
                fdt.end();
            }
            fdt.end();
        }
        fdt.end();

        return fdt.finish();
    }

  private:
    size_t size_;
    unsigned char *start_;
    Span<const uint8_t> initramfs_;
    unsigned char *dtb_;
    LinuxMachine *vm_;
};

class EPOS_Launcher {
  public:
    EPOS_Launcher(size_t size, const Span<const uint8_t> &epos, Thread::Criterion criterion)
        : epos_(epos),
          buffer_(static_cast<unsigned char *>(Memory::alloc(size)), size),
          thread_(worker, this, criterion) {}

  private:
    static void *worker(void *pointer) {
        typedef Meta::GetFromTypeList<Traits<Ethernet>::Devices, 0>::Result NetworkDevice;
        typedef virtio::Network<VirtualSwitch<NetworkDevice>, 0x30200000> Network;
        typedef Meta::GetFromTypeList<Traits<UART>::Devices, 0>::Result SerialDevice;
        typedef virtio::Console<SerialDevice, 0x30000000> Serial;

        auto *self = reinterpret_cast<EPOS_Launcher *>(pointer);
        memcpy(self->buffer_.data(), self->epos_.data(), self->epos_.length());
        auto *vm = new GenericVirtualMachine<Serial, Network>(self->buffer_.data(), self->buffer_.length());
        vm->activate(1, 15);
        return nullptr;
    }

  private:
    const Span<const uint8_t> &epos_;
    Span<uint8_t> buffer_;
    Thread thread_;
};

int main() {
    typedef Meta::GetFromTypeList<Traits<Ethernet>::Devices, 0>::Result Device;

    auto *link = new LinkIPv4ToEthernet(*Device::instance());
    auto *ipv4 = new IPv4(IPv4::Address(192, 168, 1, 167), *link);
    auto *udp  = new UDP(ipv4);
    auto *tftp = new TFTP(*udp);

    Receiver receiver(tftp);

    const auto &linux     = receiver.linux();
    const auto &initramfs = receiver.initramfs();
    const auto &epos      = receiver.epos();

    LinuxLauncher vm0(128 * 1024 * 1024, linux, initramfs, Thread::Criterion(Thread::Criterion::NORMAL, 1));
    EPOS_Launcher vm1(128 * 1024 * 1024, epos, Thread::Criterion(Thread::Criterion::NORMAL, 1));

    while (1)
        Alarm::udelay(100'000'000);
}

//// class EPOS_Launcher {
////   public:
////     EPOS_Launcher(const Chunk &chunk,
////                                 Thread::Criterion criterion = Thread::Criterion(Thread::Criterion::NORMAL, 0))
////         : chunk_(chunk),
////           thread_(worker, this, criterion) {}
////
////   private:
////     static void *worker(void *pointer) {
////         typedef Meta::GetFromTypeList<Traits<Ethernet>::Devices, 0>::Result NetworkDevice;
////         typedef virtio::Network<VirtualSwitch<NetworkDevice>, 0x30200000> Network;
////         typedef Meta::GetFromTypeList<Traits<UART>::Devices, 0>::Result SerialDevice;
////         typedef virtio::Console<SerialDevice, 0x30000000> Serial;
////
////         auto *self = reinterpret_cast<EPOS_Launcher *>(pointer);
////
////         auto *epos = static_cast<unsigned char *>(Memory::alloc(VirtualMachineMemorySize));
////
////         memcpy(epos, reinterpret_cast<void *>(self->chunk_.start()), self->chunk_.size());
////
////         auto *vm = new GenericVirtualMachine<Serial, Network>(epos, VirtualMachineMemorySize);
////
////         vm->activate(1, 15);
////         return nullptr;
////     }
////
////   private:
////     const Chunk &chunk_;
////     Thread thread_;
//// };
//
// class Linux_VirtualMachineLauncher {
//    typedef Meta::GetFromTypeList<Traits<UART>::Devices, 0>::Result SerialDevice;
//    typedef virtio::Console<SerialDevice, 0x30000000> Serial;
//
//  public:
//    Linux_VirtualMachineLauncher(const Chunk &kernel,
//                                 const Chunk &initramfs,
//                                 const Chunk &dtb,
//                                 Thread::Criterion criterion)
//        : kernel_(kernel),
//          initramfs_(initramfs),
//          dtb_(dtb),
//          thread_(worker, this, criterion) {}
//
//  private:
//    static void *worker(void *pointer) {
//        auto *self = reinterpret_cast<Linux_VirtualMachineLauncher *>(pointer);
//
//        auto *buffer           = static_cast<unsigned char *>(Memory::alloc(VirtualMachineMemorySize));
//        uintptr_t base         = reinterpret_cast<uintptr_t>(buffer);
//        unsigned char *current = buffer;
//
//        memcpy(current, reinterpret_cast<void *>(self->kernel_.start()), self->kernel_.size());
//        current += self->kernel_.size();
//
//        current = align(current, MB);
//        memcpy(current, reinterpret_cast<void *>(self->dtb_.start()), self->dtb_.size());
//        DeviceTree *dtb = reinterpret_cast<DeviceTree *>(current);
//        ERROR(!dtb->valid(), "Invalid Device Tree!\n");
//        current += self->dtb_.size();
//
//        current                  = align(current, MB);
//        unsigned char *initramfs = current;
//        memcpy(initramfs, reinterpret_cast<void *>(self->initramfs_.start()), self->initramfs_.size());
//        current += self->initramfs_.size();
//
//        // Memory
//        unsigned int regs[4];
//        regs[0] = CPU::htobe32(base >> 32);
//        regs[1] = CPU::htobe32(base);
//        regs[2] = CPU::htobe32(VirtualMachineMemorySize >> 32);
//        regs[3] = CPU::htobe32(VirtualMachineMemorySize);
//        dtb->edit("memory@0", "reg", regs, sizeof(regs));
//
//        // Initramfs
//        regs[0] = CPU::htobe32(reinterpret_cast<uintptr_t>(initramfs) >> 32);
//        regs[1] = CPU::htobe32(reinterpret_cast<uintptr_t>(initramfs));
//        regs[2] = CPU::htobe32(reinterpret_cast<uintptr_t>(initramfs + self->initramfs_.size()) >> 32);
//        regs[3] = CPU::htobe32(reinterpret_cast<uintptr_t>(initramfs + self->initramfs_.size()));
//        dtb->edit("chosen", "linux,initrd-start", regs, sizeof(regs[0]) * 2);
//        dtb->edit("chosen", "linux,initrd-end", &regs[2], sizeof(regs[0]) * 2);
//
//        // Console
//        unsigned int irq = CPU::htobe32(Serial::IRQ);
//        regs[0]          = CPU::htobe32(Serial::Address >> 32);
//        regs[1]          = CPU::htobe32(Serial::Address);
//        regs[2]          = CPU::htobe32(0x0);
//        regs[3]          = CPU::htobe32(0x1000);
//        dtb->edit("virtio_mmio@1", "compatible", "virtio,mmio", sizeof("virtio,mmio"));
//        dtb->edit("virtio_mmio@1", "reg", regs, sizeof(regs));
//        dtb->edit("virtio_mmio@1", "interrupts", &irq, sizeof(irq));
//
//        Console::print("\n *** Linux ***\n");
//        auto *vm = new GenericVirtualMachine<Serial>(buffer, VirtualMachineMemorySize);
//        vm->activate(0, dtb);
//        return nullptr;
//    }
//
//  private:
//    static unsigned char *align(unsigned char *p, long a) {
//        uintptr_t addr = reinterpret_cast<uintptr_t>(p);
//        addr           = (addr + a - 1) & ~(a - 1);
//        return reinterpret_cast<unsigned char *>(addr);
//    }
//
//  private:
//    const Chunk &kernel_;
//    const Chunk &initramfs_;
//    const Chunk &dtb_;
//    Thread thread_;
//};
//
// int main() {
//    typedef Meta::GetFromTypeList<Traits<Ethernet>::Devices, 0>::Result Device;
//
//    auto *link = new LinkIPv4ToEthernet(*Device::instance());
//    auto *ipv4 = new IPv4(IPv4::Address(192, 168, 1, 167), *link);
//    auto *udp  = new UDP(ipv4);
//    auto *tftp = new TFTP(*udp);
//
//    Receiver receiver(tftp);
//    const Chunk &linux     = receiver.linux();
//    const Chunk &initramfs = receiver.initramfs();
//    const Chunk &dtb       = receiver.dtb();
//
//    // EPOS_Launcher vm0(receiver.epos(), Thread::Criterion(Thread::Criterion::NORMAL, 1));
//
//    Linux_VirtualMachineLauncher vm0(linux, initramfs, dtb, Thread::Criterion(Thread::Criterion::NORMAL, 1));
//    // Linux_VirtualMachineLauncher vm1(linux, initramfs, dtb, Thread::Criterion(Thread::Criterion::NORMAL, 1));
//
//    while (1)
//        Alarm::udelay(1'000'000);
//
//    return 0;
//}
