#ifndef __DEPOS_I2C__
#define __DEPOS_I2C__

namespace DEPOS {

class I2C_Controller {
  public:
    virtual ~I2C_Controller() = default;
    virtual bool read(uint8_t, void *, size_t, bool);
    virtual bool write(uint8_t, const void *, size_t, bool);
};

} // namespace DEPOS

#endif
