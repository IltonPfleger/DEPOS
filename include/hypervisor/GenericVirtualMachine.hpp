#pragma once

#include <Meta.hpp>
#include <architecture/VirtualCPU.hpp>
#include <hypervisor/VirtualInterruptController.hpp>
#include <hypervisor/VirtualMachine.hpp>
#include <types.hpp>

namespace DEPOS {

template <typename... Devices> class GenericVirtualMachine : public VirtualMachine {
    template <typename... D> struct DeviceCollection {
        DeviceCollection(VirtualMachine &, VirtualCPU &) {}
        bool read(uintptr_t, unsigned int *) { return false; }
        bool write(uintptr_t, unsigned int) { return false; }
        void interrupt(unsigned int) {}
    };

    template <typename Head, typename... Tail> struct DeviceCollection<Head, Tail...> {
        Head _device;
        DeviceCollection<Tail...> _others;

        DeviceCollection(VirtualMachine &machine, VirtualCPU &cpu)
            : _device(create(machine, cpu)),
              _others(machine, cpu) {}

        static Head create(VirtualMachine &machine, VirtualCPU &cpu) {
            if constexpr (Meta::IsBaseOf<VirtualInterruptController, Head>::Result) {
                return Head(cpu);
            } else {
                return Head(machine);
            }
        }

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

        void interrupt(unsigned int id) {
            if constexpr (Meta::IsBaseOf<VirtualInterruptController, Head>::Result) {
                _device.interrupt(id);
            } else {
                _others.interrupt(id);
            }
        }
    };

  public:
    GenericVirtualMachine(void *entry, size_t size)
        : VirtualMachine(Chunk(entry, size)),
          cpu_(this),
          devices_(*this, cpu_) {}

    template <typename... Args> void boot(Args... args) { cpu_.boot(args...); }
    bool read(uintptr_t target, unsigned int *destination) override { return devices_.read(target, destination); }
    bool write(uintptr_t target, unsigned int source) override { return devices_.write(target, source); }
    void interrupt(unsigned int id) override { devices_.interrupt(id); }

  private:
    VirtualCPU cpu_;
    DeviceCollection<Devices...> devices_;
};

} // namespace DEPOS
