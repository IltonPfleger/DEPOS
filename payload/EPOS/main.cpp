#include <Thread.hpp>
#include <Traits.hpp>
#include <architecture/CPU.hpp>
#include <architecture/riscv64/VirtualPLIC.hpp>
#include <hypervisor/GenericVirtualMachine.hpp>
#include <hypervisor/VirtualSwitch.hpp>
#include <hypervisor/dtb/FDT_Builder.hpp>
#include <hypervisor/virtio/Console.hpp>
#include <hypervisor/virtio/Network.hpp>
#include <machine/Machine.hpp>
#include <utility/Console.hpp>
#include <utility/Delay.hpp>
#include <utility/Span.hpp>

#define ___STR(X) #X
#define __STR(X) ___STR(X)

using namespace QUARK;

constexpr size_t MB = 1024 * 1024;

struct Image {
    alignas(2 * MB) static constexpr unsigned char EPOS[] = {
#include __STR(__EPOS__)
    };
};

class DummyNetwork : public Observed<const NetworkBuffer *> {
  public:
    using Observer = QUARK::Observer<const NetworkBuffer *>;
    static auto instance() {
        static DummyNetwork instance;
        return &instance;
    }
};

class EPOS_Launcher {
    using Network             = virtio::Network<VirtualSwitch<DummyNetwork>, 0x30200000, 50>;
    using SerialDevice        = Meta::GetFromTypeList<Traits<UART>::Devices, 0>::Result;
    using Serial              = virtio::Console<SerialDevice, 0x30000000>;
    using InterruptController = VirtualPLIC<0xc000000>;
    using EPOS_Machine        = GenericVirtualMachine<InterruptController, Serial, Network>;

  public:
    EPOS_Launcher(size_t size, Span<const uint8_t> epos, Thread::Criterion criterion)
        : epos_(epos),
          buffer_(static_cast<unsigned char *>(Memory::alloc(size)), size),
          thread_(worker, this, criterion),
          machine_(buffer_.data(), buffer_.length()) {}

  private:
    static void *worker(void *pointer) {
        auto *self = reinterpret_cast<EPOS_Launcher *>(pointer);
        memcpy(self->buffer_.data(), self->epos_.data(), self->epos_.length());
        self->machine_.boot(1);
        return nullptr;
    }

  private:
    const Span<const uint8_t> epos_;
    Span<uint8_t> buffer_;
    Thread thread_;
    EPOS_Machine machine_;
};

int main() {
    TraceIn();

    Span<const uint8_t> epos(static_cast<const uint8_t *>(Image::EPOS), sizeof(Image::EPOS));

    new EPOS_Launcher(128 * 1024 * 1024, epos, Thread::Criterion(Thread::Criterion::NORMAL, 1));
    new EPOS_Launcher(128 * 1024 * 1024, epos, Thread::Criterion(Thread::Criterion::NORMAL, 1));
    new EPOS_Launcher(128 * 1024 * 1024, epos, Thread::Criterion(Thread::Criterion::NORMAL, 1));
    new EPOS_Launcher(128 * 1024 * 1024, epos, Thread::Criterion(Thread::Criterion::NORMAL, 2));
    new EPOS_Launcher(128 * 1024 * 1024, epos, Thread::Criterion(Thread::Criterion::NORMAL, 2));
    new EPOS_Launcher(128 * 1024 * 1024, epos, Thread::Criterion(Thread::Criterion::NORMAL, 2));

    return 0;
}
