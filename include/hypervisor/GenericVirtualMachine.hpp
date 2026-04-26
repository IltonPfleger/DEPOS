#pragma once

#include <Meta.hpp>
#include <architecture/VCPU.hpp>
#include <hypervisor/VirtualMachine.hpp>
#include <types.hpp>

namespace DEPOS {

template <typename... Devices> class GenericVirtualMachine : public VirtualMachine {
    template <typename... D> struct DeviceCollection {
        DeviceCollection(VirtualMachine *) {}
        bool read(uintptr_t, uint32_t *) { return false; }
        bool write(uintptr_t, uint32_t) { return false; }
    };

    template <typename Head, typename... Tail>
    struct DeviceCollection<Head, Tail...> : public DeviceCollection<Tail...> {
        Head m_device;

        using Base = DeviceCollection<Tail...>;

        DeviceCollection(VirtualMachine *vm)
            : Base(vm),
              m_device(vm) {}

        bool read(uintptr_t target, uint32_t *dest) {
            if (m_device.read(target, dest)) return true;
            return Base::read(target, dest);
        }

        bool write(uintptr_t target, uint32_t src) {
            if (m_device.write(target, src)) return true;
            return Base::write(target, src);
        }
    };

  public:
    GenericVirtualMachine(void *entry, size_t size)
        : m_devices(this),
          m_cpu(this, reinterpret_cast<uintptr_t>(entry), size) {}

    template <typename... Args> void start(Args... args) { m_cpu.start(args...); }

    virtual bool read(uintptr_t target, unsigned int *destination) override {
        return m_devices.read(target, destination);
    }

    virtual bool write(uintptr_t target, unsigned int source) override { return m_devices.write(target, source); }

    virtual void interrupt(unsigned int id) override { m_cpu.interrupt(id); }

  private:
    DeviceCollection<Devices...> m_devices;
    VCPU m_cpu;
};

} // namespace DEPOS
