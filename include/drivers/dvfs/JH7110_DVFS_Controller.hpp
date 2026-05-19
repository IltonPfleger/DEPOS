#ifndef __DEPOS_JH7110_DVFS_CONTROLLER__
#define __DEPOS_JH7110_DVFS_CONTROLLER__

#include <Alarm.hpp>
#include <architecture/Timer.hpp>
#include <drivers/clock/JH7110_Clock_Controller.hpp>
#include <drivers/dvfs/DVFS_Controller.hpp>
#include <drivers/i2c/DesignWare_I2C_Controller.hpp>
#include <drivers/pmic/AXP15060_Controller.hpp>

namespace DEPOS {

class JH7110_DVFS_Controller : public DVFS_Controller {

    static constexpr PState States[] = {
        {1500000000, 1040000},
        {750000000, 900000},
        {500000000, 900000},
        {375000000, 900000},
    };

    static constexpr PStateTable Available = {(sizeof(States) / sizeof(PState)), States};

  public:
    JH7110_DVFS_Controller()
        : i2c_(),
          pmic_(i2c_) {}

    virtual bool set(const PState &pstate) override {
        PState selected;
        bool found = false;

        for (auto &i : States) {
            if (i.frequency == pstate.frequency && i.voltage == pstate.voltage) {
                selected = i;
                found    = true;
            }
        }

        if (!found) return false;

        uint32_t divisor = 1500000000 / selected.frequency;

        if (!pmic_.voltage(2, pstate.voltage)) return false;

        Timer::uspin(1);

        JH7110_Clock_Controller::divide(JH7110_Clock_Controller::SYSCRG_CLK_CPU_CORE, divisor);

        return true;
    }

    virtual const PStateTable &available() override { return Available; }

    uintmax_t voltage() { return pmic_.voltage(2); }

    uintmax_t clock() {
        return 1500000000 / JH7110_Clock_Controller::divisor(JH7110_Clock_Controller::SYSCRG_CLK_CPU_CORE);
    }

  private:
    DesignWare_I2C_Controller<I2C5> i2c_;
    AXP15060_Controller<PMIC0> pmic_;
};

} // namespace DEPOS

#endif
