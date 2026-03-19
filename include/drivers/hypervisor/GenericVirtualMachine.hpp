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
    GenericVirtualMachine(uintptr_t memory, size_t size)
        : m_memory(memory),
          m_size(size),
          m_devices(this) {}

    template <typename... Args> void start(void (*entry)(Args...), Args... args) {
        new (&m_cpu) VirtualCPU(entry, m_memory, m_size, this, args...);
    }

    virtual bool read(uintptr_t target, unsigned int *destination) override {
        return m_devices.read(target, destination);
    }

    virtual bool write(uintptr_t target, unsigned int source) override { return m_devices.write(target, source); }

    virtual void interrupt(unsigned int id) override { m_cpu.interrupt(id); }

  private:
    uintptr_t m_memory;
    size_t m_size;
    DeviceCollection<Devices...> m_devices;
    VirtualCPU m_cpu;
};

} // namespace DEPOS
