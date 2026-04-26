#pragma once

#include <Traits.hpp>
#include <architecture/CPU.hpp>
#include <architecture/VCPU.hpp>
#include <hypervisor/virtio/Handler.hpp>
#include <shared/libraries/libc/string.h>
#include <utils/Observer.hpp>

namespace DEPOS {

namespace virtio {

template <typename Device, uintptr_t Base>
class Console : public Handler, public Observer<const unsigned char *, size_t> {
    enum { RX = 0, TX = 1 };

  public:
    void notify(unsigned int source) {
        if (source != TX) return;
        auto &queue = this->m_queues[source];
        queue.process([](uintptr_t data, size_t length) {
            for (size_t i = 0; i < length; i++)
                DEPOS::Console::print(((char *)data)[i]);
        });
    }

    void update(const unsigned char *buffer, size_t length) override {
        auto &queue = this->m_queues[RX];
        queue.produce(length, [&](uint8_t *destination) { memcpy(destination, buffer, length); });
        this->interrupt() |= 1;
        m_owner->interrupt(IRQ);
    }

    Console(VirtualMachine *owner)
        : m_owner(owner) {
        this->m_header.m_magic                     = ('t' << 24) | ('r' << 16) | ('i' << 8) | 'v';
        this->m_header.m_version                   = 1;
        this->m_header.m_id                        = 3;
        this->m_header.m_vendor                    = 0x554d4551;
        this->m_header.m_host_features             = 1 << 27;
        this->m_header.m_max_number_of_descriptors = k_number;
        Device::instance()->attach(this);
    }

  public:
    static constexpr uintptr_t Address = Base;
    static constexpr int IRQ           = 32;
    static constexpr size_t Size       = sizeof(LegacyHeader);

  private:
    static const int k_number = 32;

  private:
    VirtualMachine *m_owner;
};

} // namespace virtio

} // namespace DEPOS
