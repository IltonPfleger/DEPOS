#ifndef __DEPOS_AXP10060__
#define __DEPOS_AXP10060__

#include <drivers/i2c/I2C_Controller.hpp>
#include <drivers/pmic/PMIC_Controller.hpp>

namespace DEPOS {

template <typename Tag> class AXP15060_Controller : public PMIC_Controller {
    using Traits = DEPOS::Traits<Tag>;

    enum { DCDC1_VOLTAGE_CONTROL = 0x13 };

  public:
    AXP15060_Controller(I2C_Controller &i2c)
        : i2c_(i2c) {}

    virtual bool voltage(unsigned int rail, uint32_t microvolts) {
        const unsigned int *range = nullptr;
        for (auto i : Traits::Voltages) {
            if (i[0] == rail) range = i + 1;
        }

        if (microvolts < range[0] || microvolts > range[1]) return false;

        unsigned char data = 0;

        if (microvolts <= 1210) {
            data = (microvolts - 500) / 10;
        } else {
            data = 71 + ((microvolts - 1220) / 20);
        }

        char payload[2];

        payload[0] = DCDC1_VOLTAGE_CONTROL + rail;
        payload[1] = data;

        i2c_.write(Traits::Address, payload, 2, true);

        return true;
    }

    virtual uint32_t voltage(unsigned int rail) {
        char target = DCDC1_VOLTAGE_CONTROL + rail;
        char data   = 0;

        if (!i2c_.write(Traits::Address, &target, 1, false)) return 0;

        if (!i2c_.read(Traits::Address, &data, 1, true)) return 0;

        unsigned int value = data & 0x7F;

        if (value < 71) {
            return 500 + (value * 10);
        } else {
            return 1220 + ((value - 71) * 20);
        }
    }

  private:
    I2C_Controller &i2c_;
};

} // namespace DEPOS

#endif
