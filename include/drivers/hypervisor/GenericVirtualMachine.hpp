#pragma once

#include <Meta.hpp>
#include <drivers/hypervisor/VirtualMachine.hpp>
#include <types.hpp>

namespace DEPOS {

template <typename... Devices> class GenericVirtualMachine : public VirtualMachine {
    template <typename... D> struct DeviceCollection {
        bool read(uintptr_t, unsigned int *) { return false; }
        bool write(uintptr_t, unsigned int) { return false; }
    };

    template <typename Head, typename... Tail> struct DeviceCollection<Head, Tail...> {
        Head device;
        DeviceCollection<Tail...> rest;

        bool read(uintptr_t target, unsigned int *destination) {
            if (device.read(target, destination)) {
                return true;
            }
            return rest.read(target, destination);
        }

        bool write(uintptr_t target, unsigned int source) {
            if (device.write(target, source)) {
                return true;
            }
            return rest.write(target, source);
        }
    };

  public:
    GenericVirtualMachine() = default;
    virtual bool read(uintptr_t target, unsigned int *destination) override {
        return m_devices.read(target, destination);
    }
    virtual bool write(uintptr_t target, unsigned int source) override { return m_devices.write(target, source); }

  private:
    DeviceCollection<Devices...> m_devices;
};

} // namespace DEPOS
