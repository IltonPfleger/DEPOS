#pragma once

#include <Meta.hpp>
#include <abstractions/VirtualCPU.hpp>
#include <drivers/hypervisor/VirtualMachine.hpp>
#include <types.hpp>

namespace DEPOS {

template <typename... Devices> class GenericVirtualMachine : public VirtualMachine {
    template <typename... D> struct DeviceCollection {
        DeviceCollection(VirtualMachine *) {}
        bool read(uintptr_t, unsigned int *) { return false; }
        bool write(uintptr_t, unsigned int) { return false; }
    };

    template <typename Head, typename... Tail> struct DeviceCollection<Head, Tail...> {
        Head m_device;
        DeviceCollection<Tail...> m_others;

        DeviceCollection(VirtualMachine *machine)
            : m_device(machine),
              m_others(machine) {}

        bool read(uintptr_t target, unsigned int *destination) {
            if (m_device.read(target, destination)) {
                return true;
            }
            return m_others.read(target, destination);
        }

        bool write(uintptr_t target, unsigned int source) {
            if (m_device.write(target, source)) {
                return true;
            }
            return m_others.write(target, source);
        }
    };

  public:
    template <typename... Args>
    GenericVirtualMachine(void (*entry)(Args...), uintptr_t memory, size_t size, Args... args)
        : m_devices(this) {
        m_cpu = new VirtualCPU(entry, memory, size, this, args...);
    };

    virtual bool read(uintptr_t target, unsigned int *destination) override {
        return m_devices.read(target, destination);
    }

    virtual bool write(uintptr_t target, unsigned int source) override { return m_devices.write(target, source); }

    virtual void interrupt(unsigned int id) override { m_cpu->interrupt(id); }

  private:
    DeviceCollection<Devices...> m_devices;
    VirtualCPU *m_cpu;
};

} // namespace DEPOS
