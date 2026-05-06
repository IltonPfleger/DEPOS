#ifndef __DEPOS_NETWORK_BUFFER_HEADER__
#define __DEPOS_NETWORK_BUFFER_HEADER__

#include <network/NetworkProtocolIdentifier.hpp>

namespace DEPOS {

class NetworkBuffer {
  public:
    NetworkBuffer(void *start, size_t size, NetworkProtocolIdentifier protocol = NetworkProtocolIdentifier::UNKNOWN())
        : _start(reinterpret_cast<uint8_t *>(start)),
          _size(size),
          _data(_start),
          _length(_size),
          _protocol(protocol),
          _references(0) {}

    template <typename T = uint8_t *> [[nodiscard]] T data(this auto &&self) { return reinterpret_cast<T>(self._data); }

    void length(size_t l) { _length = l; }

    void references(size_t r) { _references = r; }

    void protocol(size_t p) { _protocol = p; }

    [[nodiscard]] size_t protocol() const { return _protocol; }

    [[nodiscard]] size_t references() const { return _references; }

    [[nodiscard]] size_t length() const { return _length; }

    [[nodiscard]] uint8_t *start() const { return _start; }

    [[nodiscard]] size_t size() const { return _size; }

    [[nodiscard]] size_t offset() const { return static_cast<size_t>(data() - start()); }

    [[nodiscard]] size_t remaining() const { return size() - offset(); }

    void reset() { _data = _start; }

    bool advance(size_t bytes) {
        // if (bytes > remaining()) return false;
        _data += bytes;
        return true;
    }

    bool rewind(size_t bytes) {
        // if (bytes > offset()) return false;
        _data -= bytes;
        return true;
    }

  private:
    uint8_t *const _start;
    const size_t _size;

    uint8_t *_data;
    size_t _length;

    NetworkProtocolIdentifier _protocol;

    volatile mutable size_t _references;
};

} // namespace DEPOS

#endif
