#ifndef __DEPOS_NETWORK_BUFFER__
#define __DEPOS_NETWORK_BUFFER__

namespace DEPOS {

class NetworkBuffer {
  public:
    struct InternalData {
        uint8_t *start;
        size_t size;
        volatile mutable size_t references = 0;
    };

    NetworkBuffer(InternalData *data = 0, uint16_t protocol = 0)
        : internal_(data),
          protocol_(protocol) {
        if (internal()) {
            data_   = start();
            length_ = size();
        }
    }

    template <typename T = uint8_t *> [[nodiscard]] T data(this auto &&self) { return reinterpret_cast<T>(self.data_); }
    [[nodiscard]]
    uint8_t operator[](this auto &&self, size_t i) {
        return self.data_[i];
    }

    [[nodiscard]]
    const InternalData *internal() const {
        return internal_;
    }

    [[nodiscard]]
    size_t protocol() const {
        return protocol_;
    }

    [[nodiscard]]
    size_t length() const {
        return length_;
    }

    [[nodiscard]]
    auto &references() {
        return internal()->references;
    }

    [[nodiscard]]
    uint8_t *start() const {
        return internal()->start;
    }

    [[nodiscard]]
    size_t size() const {
        return internal()->size;
    }

    [[nodiscard]]
    size_t offset() const {
        return static_cast<size_t>(data() - start());
    }

    [[nodiscard]]
    size_t remaining() const {
        return size() - offset();
    }

    void length(size_t l) { length_ = l; }

    void protocol(size_t p) { protocol_ = p; }

    void reset() { data_ = start(); }

    bool advance(size_t bytes) {
        data_ += bytes;
        return true;
    }

    bool rewind(size_t bytes) {
        data_ -= bytes;
        return true;
    }

  private:
    const InternalData *internal_;
    uint8_t *data_;
    size_t length_;
    uint16_t protocol_;
};

} // namespace DEPOS

#endif
