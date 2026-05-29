#ifndef __DEPOS_FDT_PROPERTY__
#define __DEPOS_FDT_PROPERTY__

namespace DEPOS {

struct FDT_Property {
    uint32_t len;
    uint32_t nameoff;
    char data[];
};

} // namespace DEPOS

#endif
