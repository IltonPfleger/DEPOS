#ifndef __DEPOS_MONITOR__
#define __DEPOS_MONITOR__

#include <Traits.hpp>
#include <architecture/PMU.hpp>

namespace DEPOS {

class Monitor {
    struct Snapshot {
        uintmax_t local[Traits<CPU>::Active][PMU::L_LAST - PMU::L_FIRST];
        uintmax_t global[PMU::G_LAST - PMU::G_FIRST];
        uintmax_t timestamp;
    };

  public:
    static void onTick() {
        size_t core = CPU::id();

        for (PMU::Event i = PMU::L_FIRST; i < PMU::L_LAST; ++i) {
            snapshot().local[core][i] = PMU::read(i);
        }

        if (core == Traits<CPU>::BSP) {
            for (PMU::Event i = PMU::G_FIRST; i < PMU::G_LAST - PMU::G_FIRST; ++i) {
                snapshot().global[i - PMU::G_FIRST] = PMU::read(i);
            }
            index_++;
        }
    }

  private:
    static Snapshot &snapshot() { return snapshots_[index_ % Length]; };

  private:
    static constexpr size_t Length        = Traits<Monitor>::Length;
    static constinit inline size_t index_ = 0;
    static inline Snapshot snapshots_[Length];
};

} // namespace DEPOS

#endif
