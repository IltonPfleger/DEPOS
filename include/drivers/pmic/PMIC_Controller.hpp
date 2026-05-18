#ifndef __DEPOS_PMIC__
#define __DEPOS_PMIC__

namespace DEPOS {

class PMIC_Controller {
  public:
    virtual ~PMIC_Controller()                          = default;
    virtual bool voltage(uint32_t, uint32_t microvolts) = 0;
    virtual uint32_t voltage(uint32_t)                  = 0;
};

} // namespace DEPOS

#endif
