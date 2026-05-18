#ifndef __DEPOS_JH7110_DVFS_CONTROLLER__
#define __DEPOS_JH7110_DVFS_CONTROLLER__

#include <Alarm.hpp>
#include <architecture/Timer.hpp>
#include <drivers/dvfs/DVFS_Controller.hpp>
#include <drivers/i2c/DesignWare_I2C_Controller.hpp>
#include <drivers/pmic/AXP15060_Controller.hpp>

namespace DEPOS {

class JH7110_DVFS_Controller : public DVFS_Controller {
    static constexpr uint64_t Frequencys[] = {375000000, 500000000, 750000000, 1500000000};

    enum { ROOT = 0x000, DIV = 0x004 };

  public:
    JH7110_DVFS_Controller()
        : i2c_(),
          pmic_(i2c_) {}

    virtual bool set(const PState &&pstate) override {
        uint32_t div = Frequencys[3] / pstate.frequency;

        if (div > 7) return false;
        if (!pmic_.voltage(2, pstate.voltage)) return false;

        Timer::uspin(1);

        reg32(DIV) = div;

        return true;
    }

  private:
    static volatile uint32_t &reg32(uint32_t offset) {
        return *reinterpret_cast<volatile uint32_t *>(Traits<MemoryMap>::SYSCRG + offset);
    }

  private:
    DesignWare_I2C_Controller<I2C5> i2c_;
    AXP15060_Controller<PMIC0> pmic_;
};

} // namespace DEPOS

#endif
