#ifndef __DEPOS_NETWORK_BUFFER__
#define __DEPOS_NETWORK_BUFFER__

namespace DEPOS {

class NetworkBuffer {
  public:
    NetworkBuffer(void *start, size_t size, uint16_t protocol = 0)
        : start_(static_cast<uint8_t *>(start)),
          size_(size),
          data_(start_),
          length_(0),
          protocol_(protocol) {}

    template <typename T = uint8_t *> [[nodiscard]] T data(this auto &&self) { return reinterpret_cast<T>(self.data_); }

    [[nodiscard]]
    uint8_t operator[](this auto &&self, size_t i) {
        return self.data_[i];
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
    uint8_t *start() const {
        return start_;
    }

    [[nodiscard]]
    size_t size() const {
        return size_;
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
    uint8_t *const start_;
    size_t size_;

    uint8_t *data_;
    size_t length_;

    uint16_t protocol_;
};

} // namespace DEPOS

#endif
