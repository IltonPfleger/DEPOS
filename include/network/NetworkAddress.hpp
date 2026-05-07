#ifndef __DEPOS_NETWORK_ADDRESS_HEADER__
#define __DEPOS_NETWORK_ADDRESS_HEADER__

#include <libraries/libc/string.h>
#include <types.hpp>

namespace DEPOS {

class NetworkAddress {
  public:
    NetworkAddress(const NetworkAddress &)            = default;
    NetworkAddress &operator=(const NetworkAddress &) = default;

    NetworkAddress() = default;

    NetworkAddress(const uint8_t *data, size_t length)
        : _data(data),
          _length(length) {}

    [[nodiscard]] constexpr operator unsigned char *(this auto &&self) { return self._data; }

    [[nodiscard]] uint8_t operator[](this auto &&self, size_t i) { return self._data[i]; }

    [[nodiscard]] size_t length() const { return _length; }

    [[nodiscard]] bool operator==(const NetworkAddress &other) const {
        return (_length == other._length) && (memcmp(_data, other._data, _length) == 0);
    }

    [[nodiscard]] bool operator!=(const NetworkAddress &other) const { return !(*this == other); }

  private:
    const uint8_t *_data;
    size_t _length;
};

} // namespace DEPOS

#endif
