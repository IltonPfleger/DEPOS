#ifndef __DEPOS_DVFS__
#define __DEPOS_DVFS__

namespace DEPOS {

class DVFS_Controller {
  public:
    struct PState {
        uint32_t frequency;
        uint32_t voltage;
    };

    struct PStateTable {
        size_t length;
        const PState *pstates;
    };

    virtual ~DVFS_Controller()             = default;
    virtual bool set(const PState &)       = 0;
    virtual const PStateTable &available() = 0;
};

} // namespace DEPOS

#endif
