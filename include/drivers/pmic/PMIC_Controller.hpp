#ifndef __QUARK_PMIC__
#define __QUARK_PMIC__

namespace QUARK {

class PMIC_Controller {
  public:
    virtual ~PMIC_Controller()                          = default;
    virtual bool voltage(uint32_t, uint32_t microvolts) = 0;
    virtual uint32_t voltage(uint32_t)                  = 0;
};

} // namespace QUARK

#endif
