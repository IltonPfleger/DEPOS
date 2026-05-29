#ifndef __DEPOS_FDT_RESERVE_ENTRY__
#define __DEPOS_FDT_RESERVE_ENTRY__

namespace DEPOS {

struct FDT_ReserveEntry {
    uint64_t address;
    uint64_t size;
};

} // namespace DEPOS

#endif
