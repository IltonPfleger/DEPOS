#include <Traits.hpp>

class SiFiveCLINT {
    static constexpr const unsigned long long CLOCK = 1'000'000;
    static constexpr const uintptr_t ADDR           = 0x02000000;
    static constexpr const uintptr_t MTIMECMP       = ADDR + 0x4000;
    static volatile inline uintmax_t *MTIME         = reinterpret_cast<volatile uintmax_t *>(ADDR + 0xBFF8);

   public:
    static void reset(auto core) {
        static constexpr uintmax_t ticks = CLOCK / Traits::Timer::Frequency;
        auto counter                     = reinterpret_cast<volatile uintmax_t *>(MTIMECMP);
        counter[core]                    = *MTIME + ticks;
    }
};
