#ifndef __UTILITY_ALIGN_HEADER__
#define __UTILITY_ALIGN_HEADER__

namespace DEPOS {

namespace utility {

class Align {
  public:
    [[nodiscard]] constexpr static uintptr_t down(uintptr_t address, uintptr_t align) { return address & ~(align - 1); }
};

} // namespace utility

} // namespace DEPOS

#endif
