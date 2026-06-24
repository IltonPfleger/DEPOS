#ifndef __QUARK_I2C__
#define __QUARK_I2C__

namespace QUARK {

class I2C_Controller {
  public:
    virtual ~I2C_Controller()                               = default;
    virtual bool read(uint8_t, void *, size_t, bool)        = 0;
    virtual bool write(uint8_t, const void *, size_t, bool) = 0;
};

} // namespace QUARK

#endif
