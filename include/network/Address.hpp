#pragma once

#include <types.hpp>

namespace DEPOS {

class Address {
  public:
    virtual ~Address()                                  = default;
    virtual size_t size() const                         = 0;
    virtual const unsigned char *data() const           = 0;
    virtual bool operator==(const Address &other) const = 0;
    bool operator!=(const Address &other) const { return !(*this == other); }
};

} // namespace DEPOS
