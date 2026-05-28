#include <Thread.hpp>
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

constexpr size_t MB = 1024 * 1024;

struct LinuxImage {
    alignas(2 * MB) static constexpr unsigned char Kernel[] = {
#include __STR(__KERNEL__)
    };

    alignas(1 * MB) static constexpr unsigned char Dtb[] = {
#include __STR(__DTB__)
    };

    alignas(1 * MB) static constexpr unsigned char Initramfs[] = {
#include __STR(__INITRAMFS__)
    };
};

class LinuxLauncher {
  public:
    using SerialDevice = Meta::GetFromTypeList<Traits<UART>::Devices, 0>::Result;
    using Serial       = virtio::Console<SerialDevice, 0x30000000>;
    using MyMachine    = GenericVirtualMachine<Serial>;

    LinuxLauncher(size_t size, Thread::Criterion criterion)
        : size_(size),
          start_(nullptr),
          kernel_(nullptr),
          initramfs_(nullptr),
          dtb_(nullptr),
          vm_(nullptr) {
        image();
        new Thread(worker, this, criterion);
    }

    void image() {
        start_                 = reinterpret_cast<unsigned char *>(Memory::alloc(size_));
        uintptr_t address      = reinterpret_cast<uintptr_t>(start_);
        unsigned char *current = start_;

        kernel_ = current;
        current += sizeof(LinuxImage::Kernel);

        current = align(current, MB);
        dtb_    = reinterpret_cast<DeviceTree *>(current);
        current += sizeof(LinuxImage::Dtb);
        current = align(current, MB);

        initramfs_            = current;
        size_t initramfs_size = sizeof(LinuxImage::Initramfs);
        current += initramfs_size;
        current = align(current, MB);

        memcpy(kernel_, LinuxImage::Kernel, sizeof(LinuxImage::Kernel));
        memcpy(dtb_, LinuxImage::Dtb, sizeof(LinuxImage::Dtb));
        memcpy(initramfs_, LinuxImage::Initramfs, sizeof(LinuxImage::Initramfs));

        ERROR(!dtb_->valid(), "Invalid Device Tree!\n");

        dtb(address, initramfs_size);
    }

    static void *worker(void *pointer) {
        LinuxLauncher *self = reinterpret_cast<LinuxLauncher *>(pointer);
        Console::println("\n *** Linux is at core ", CPU::id(), " ***");
        MyMachine *vm = new MyMachine(self->kernel_, self->size_);
        vm->activate(0, self->dtb_);
        return nullptr;
    }

    static unsigned char *align(unsigned char *pointer, long alignment) {
        uintptr_t address = reinterpret_cast<long>(pointer);
        address           = (address + alignment - 1) & ~(alignment - 1);
        return reinterpret_cast<unsigned char *>(address);
    }

    void dtb(uintptr_t base_address, size_t initramfs_size) {
        unsigned int irq;
        unsigned int regs[4];

        regs[0] = CPU::htobe32(base_address >> 32);
        regs[1] = CPU::htobe32(base_address);
        regs[2] = CPU::htobe32(size_ >> 32);
        regs[3] = CPU::htobe32(size_);
        dtb_->edit("memory@0", "reg", regs, sizeof(regs));

        regs[0] = CPU::htobe32(reinterpret_cast<unsigned long>(initramfs_) >> 32);
        regs[1] = CPU::htobe32(reinterpret_cast<unsigned long>(initramfs_));
        regs[2] = CPU::htobe32(reinterpret_cast<unsigned long>(initramfs_ + initramfs_size) >> 32);
        regs[3] = CPU::htobe32(reinterpret_cast<unsigned long>(initramfs_ + initramfs_size));
        dtb_->edit("chosen", "linux,initrd-start", regs, sizeof(regs[0]) * 2);
        dtb_->edit("chosen", "linux,initrd-end", &regs[2], sizeof(regs[0]) * 2);

        irq     = CPU::htobe32(Serial::IRQ);
        regs[0] = CPU::htobe32(Serial::Address >> 32);
        regs[1] = CPU::htobe32(Serial::Address);
        regs[2] = CPU::htobe32(0x0);
        regs[3] = CPU::htobe32(0x1000);
        dtb_->edit("virtio_mmio@1", "compatible", "virtio,mmio", sizeof("virtio,mmio"));
        dtb_->edit("virtio_mmio@1", "reg", regs, sizeof(regs));
        dtb_->edit("virtio_mmio@1", "interrupts", &irq, sizeof(irq));
    }

  private:
    size_t size_;
    unsigned char *start_;
    unsigned char *kernel_;
    unsigned char *initramfs_;
    DeviceTree *dtb_;
    MyMachine *vm_;
};

class Interference {
  public:
    Interference()
        : running_(true) {
        for (auto &i : threads_) {
            i = new Thread(worker, this);
        }
    }

    static void *worker(void *pointer) {
        Interference *self = reinterpret_cast<Interference *>(pointer);
        while (self->running_)
            ;
        return nullptr;
    }

  private:
    volatile bool running_;
    Thread *threads_[Traits<CPU>::Active];
};

int main() {
    TraceIn();

    // Interference i;
    LinuxLauncher vm0(128 * 1024 * 1024, Thread::Criterion(Thread::Criterion::NORMAL, 1));
    LinuxLauncher vm1(128 * 1024 * 1024, Thread::Criterion(Thread::Criterion::NORMAL, 1));

    while (1)
        ;

    return 0;
}
