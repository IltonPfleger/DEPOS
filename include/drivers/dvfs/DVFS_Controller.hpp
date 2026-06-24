#ifndef __QUARK_DVFS_CONTROLLER__
#define __QUARK_DVFS_CONTROLLER__

#include <utility/Span.hpp>

namespace QUARK {

class DVFS_Controller {
  public:
    struct PState {
        uint32_t frequency;
        uint32_t voltage;
    };

    virtual ~DVFS_Controller()             = default;
    virtual bool set(const PState &)       = 0;
    virtual uintmax_t voltage()            = 0;
    virtual uintmax_t clock()              = 0;
    virtual Span<const PState> available() = 0;
};

} // namespace QUARK

#endif
