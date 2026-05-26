#pragma once

#include <Meta.hpp>
#include <architecture/VirtualCPU.hpp>
#include <hypervisor/VirtualMachine.hpp>
#include <types.hpp>

namespace DEPOS {

template <typename... Devices> class GenericVirtualMachine : public VirtualMachine {
    template <typename... D> struct DeviceCollection {
        DeviceCollection(VirtualMachine &) {}
        bool read(uintptr_t, unsigned int *) { return false; }
        bool write(uintptr_t, unsigned int) { return false; }
    };

    template <typename Head, typename... Tail> struct DeviceCollection<Head, Tail...> {
        Head _device;
        DeviceCollection<Tail...> _others;

        DeviceCollection(VirtualMachine &machine)
            : _device(machine),
              _others(machine) {}

        bool read(uintptr_t target, unsigned int *destination) {
            if (_device.read(target, destination)) {
                return true;
            }
            return _others.read(target, destination);
        }

        bool write(uintptr_t target, unsigned int source) {
            if (_device.write(target, source)) {
                return true;
            }
            return _others.write(target, source);
        }
    };

  public:
    GenericVirtualMachine(void *entry, size_t size)
        : VirtualMachine(Chunk(entry, size)),
          devices_(*this),
          cpu_(this) {}

    template <typename... Args> void activate(Args... args) { cpu_.activate(args...); }

    virtual bool read(uintptr_t target, unsigned int *destination) override {
        return devices_.read(target, destination);
    }

    virtual bool write(uintptr_t target, unsigned int source) override { return devices_.write(target, source); }

    virtual void interrupt(unsigned int id) override { cpu_.interrupt(id); }

  private:
    DeviceCollection<Devices...> devices_;
    VirtualCPU cpu_;
};

} // namespace DEPOS
