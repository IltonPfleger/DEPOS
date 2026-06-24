#ifndef __QUARK_DESIGNWARE_I2C_CONTROLLER__
#define __QUARK_DESIGNWARE_I2C_CONTROLLER__

#include <drivers/i2c/I2C_Controller.hpp>

namespace QUARK {

template <typename Tag> class DesignWare_I2C_Controller : public I2C_Controller {
    using Traits = QUARK::Traits<Tag>;

    enum {
        ENABLE        = 0x6c, // Enable Register
        ENABLE_STATUS = 0x9c, // Enable Status Register
        CONTROL       = 0x00, // Control Register
        TARGET        = 0x04, // Target Address Register
        STATUS        = 0x70, // Status Register
        DATA          = 0x10, // Data Buffer
    };

    enum {
        READ_DATA_COMMAND = 1 << 8,
        STOP_DATA_COMMAND = 1 << 9,
        STATUS_TFNF       = 1 << 1,
        STATUS_RFNE       = 1 << 3,
        STATUS_ACTIVITY   = 1 << 0,
    };

  public:
    DesignWare_I2C_Controller()  = default;
    ~DesignWare_I2C_Controller() = default;

    virtual bool read(uint8_t target, void *buffer, size_t length, bool stop = false) override {
        char *data = reinterpret_cast<char *>(buffer);

        if (reg32(TARGET) != target) {
            end();
            reg32(TARGET) = target;
            start();
        }

        for (unsigned int i = 0; i < length; i++) {
            bool last    = (i == length - 1);
            uint32_t cmd = READ_DATA_COMMAND;

            if (last && stop) cmd |= STOP_DATA_COMMAND;
            if (!wait_tx_not_full()) return false;

            reg32(DATA) = cmd;

            if (!wait_rx_not_empty()) return false;

            data[i] = reg32(DATA) & 0xFF;
        }

        return true;
    }

    virtual bool write(uint8_t target, const void *buffer, size_t length, bool stop = false) override {
        const uint8_t *data = reinterpret_cast<const uint8_t *>(buffer);

        if (reg32(TARGET) != target) {
            end();
            reg32(TARGET) = target;
            start();
        }

        for (size_t i = 0; i < length; i++) {
            bool last = (i == length - 1);

            uint32_t cmd = data[i];

            if (last && stop) cmd |= STOP_DATA_COMMAND;
            if (!wait_tx_not_full()) return false;

            reg32(DATA) = cmd;
        }

        if (stop) {
            while (reg32(STATUS) & STATUS_ACTIVITY)
                ;
        }

        return true;
    }

  private:
    bool wait_rx_not_empty() {
        while (!(reg32(STATUS) & STATUS_RFNE))
            ;
        return true;
    }

    bool wait_tx_not_full() {
        while (!(reg32(STATUS) & STATUS_TFNF))
            ;
        return true;
    }

    void start() {
        reg32(ENABLE) = 1;
        while ((reg32(ENABLE_STATUS) & 1) == 0)
            ;
    }

    void end() {
        reg32(ENABLE) = 0;
        while ((reg32(ENABLE_STATUS) & 1) != 0)
            ;
    }

    volatile uint32_t &reg32(unsigned int offset) {
        return *reinterpret_cast<volatile uint32_t *>(Traits::Address + offset);
    }
};

} // namespace QUARK

#endif
