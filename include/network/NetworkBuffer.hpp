#ifndef __DEPOS_NETWORK_BUFFER_HEADER__
#define __DEPOS_NETWORK_BUFFER_HEADER__

#include <network/NetworkProtocolIdentifier.hpp>

namespace DEPOS {

class NetworkBuffer {
  public:
    struct InternalData {
        uint8_t *start;
        size_t size;
        volatile mutable size_t references = 0;
    };

    NetworkBuffer(InternalData *data = 0, uint16_t protocol = 0)
        : _internal(data),
          _protocol(protocol) {
        if (internal()) {
            _data   = start();
            _length = size();
        }
    }

    template <typename T = uint8_t *> [[nodiscard]] T data(this auto &&self) { return reinterpret_cast<T>(self._data); }
    [[nodiscard]] uint8_t operator[](this auto &&self, size_t i) { return self._data[i]; }

    [[nodiscard]] const InternalData *internal() const { return _internal; }

    void length(size_t l) { _length = l; }

    void protocol(size_t p) { _protocol = p; }

    [[nodiscard]] size_t protocol() const { return _protocol; }

    [[nodiscard]] size_t length() const { return _length; }

    [[nodiscard]] auto &references() { return internal()->references; }

    [[nodiscard]] uint8_t *start() const { return internal()->start; }

    [[nodiscard]] size_t size() const { return internal()->size; }

    [[nodiscard]] size_t offset() const { return static_cast<size_t>(data() - start()); }

    [[nodiscard]] size_t remaining() const { return size() - offset(); }

    void reset() { _data = start(); }

    bool advance(size_t bytes) {
        _data += bytes;
        return true;
    }

    bool rewind(size_t bytes) {
        _data -= bytes;
        return true;
    }

  private:
    const InternalData *_internal;
    uint8_t *_data;
    size_t _length;
    uint16_t _protocol;
};

} // namespace DEPOS

#endif
