#ifndef __DEPOS_NETWORK_BUFFER__
#define __DEPOS_NETWORK_BUFFER__

#include <types.hpp>

namespace DEPOS {

class NetworkBuffer {
    friend class NetworkDevice;

  public:
    NetworkBuffer(void *start, size_t head, size_t tail, uint32_t *references)
        : start_(static_cast<uint8_t *>(start)),
          head_(start_ + head),
          tail_(start_ + tail),
          references_(references) {}

    NetworkBuffer()
        : NetworkBuffer(nullptr, 0, 0, nullptr) {}

    template <typename T = uint8_t *>
    [[nodiscard]]
    T data(this auto &&self) {
        return reinterpret_cast<T>(self.head_);
    }

    template <typename T = uint8_t *>
    [[nodiscard]]
    T start(this auto &&self) {
        return reinterpret_cast<T>(self.start_);
    }

    [[nodiscard]]
    uint8_t operator[](this auto &&self, size_t i) {
        return self.head_[i];
    }

    [[nodiscard]]
    size_t references() const {
        assert(references_);
        return *references_;
    }

    [[nodiscard]]
    size_t length() const {
        return static_cast<size_t>(tail_ - start_);
    }

    [[nodiscard]]
    size_t offset() const {
        return static_cast<size_t>(head_ - start_);
    }

    bool advance(size_t bytes) {
        head_ += bytes;
        return true;
    }

    bool rewind(size_t bytes) {
        head_ -= bytes;
        return true;
    }

    bool extend(size_t bytes) {
        tail_ += bytes;
        return true;
    }

    bool shrink(size_t bytes) {
        tail_ -= bytes;
        return true;
    }

  private:
    uint8_t *const start_;

    uint8_t *head_;
    uint8_t *tail_;

    uint32_t *references_;
};

} // namespace DEPOS

#endif
