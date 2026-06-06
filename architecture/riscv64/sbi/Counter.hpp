#pragma once

#include <Semaphore.hpp>
#include <Spin.hpp>
#include <architecture/riscv64/CLINT.hpp>
#include <architecture/riscv64/Context.hpp>
#include <architecture/riscv64/VirtualCPU.hpp>

namespace DEPOS::sbi {

class Counter {
    using PageTable = MMU::PageTable;

  public:
    static constexpr unsigned int EID = 0;

    static void handler(ContextFrame *c) {
        switch (c->a6) {
            case 0: {
                c->a0 = CPU::Atomic::finc(counter_);
                break;
            }
            case 1: {
                lock_.acquire();
                uintptr_t address = PageTable::virt2phys(c->a1);
                uintptr_t *ts     = reinterpret_cast<uintptr_t *>(address);
                Console::println("DEVICE: ", c->a2);
                for (uintmax_t i = 1; i < c->a0; i += 2) {
                    Console::println(ts[i] - ts[i - 1]);
                }
                lock_.release();
                break;
            }
        }
    }

  private:
    static constinit inline size_t counter_ = 0;
    static constinit inline Spin lock_;
};

} // namespace DEPOS::sbi
