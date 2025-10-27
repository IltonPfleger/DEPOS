#include <Traits.hpp>

class SiFiveCLINT {
    static constexpr const unsigned long long Clock = 1'000'000;
    static constexpr const uintptr_t Addr           = 0x02000000;
    // static constexpr const uintptr_t MTIMECMP       = Addr + 0x4000;
    static volatile inline uintmax_t *MTIMECMP = reinterpret_cast<volatile uintmax_t *>(Addr + 0x4000);
    static volatile inline uintmax_t *MTIME    = reinterpret_cast<volatile uintmax_t *>(Addr + 0xBFF8);

   public:
    static void reset(auto core) {
        static constexpr uintmax_t ticks = Clock / Traits::Timer::Frequency;
        MTIMECMP[core] = *MTIME + ticks;
    }
};
