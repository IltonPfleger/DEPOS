#ifndef __DEPOS_DVFS__
#define __DEPOS_DVFS__

namespace DEPOS {

class DVFS_Controller {
  public:
    struct PState {
        uint32_t frequency;
        uint32_t voltage;
    };

    virtual ~DVFS_Controller()        = default;
    virtual bool set(const PState &&) = 0;
};

} // namespace DEPOS

#endif
