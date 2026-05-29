#ifndef __DEPOS_MEMORY_CHUNK__
#define __DEPOS_MEMORY_CHUNK__

#include <types.hpp>

namespace DEPOS {

class Chunk {
  public:
    constexpr Chunk()
        : start_(0),
          size_(0) {}

    constexpr Chunk(uintptr_t start, size_t size)
        : start_(start),
          size_(size) {}

    constexpr Chunk(const void *start, size_t size)
        : start_(reinterpret_cast<uintptr_t>(start)),
          size_(size) {}

    [[nodiscard]]
    constexpr uintptr_t start() const {
        return start_;
    }

    [[nodiscard]]
    constexpr size_t size() const {
        return size_;
    }

    [[nodiscard]]
    constexpr uintptr_t end() const {
        return start() + size();
    }

    [[nodiscard]]
    unsigned char *data() const {
        return reinterpret_cast<unsigned char *>(start());
    }

    [[nodiscard]]
    constexpr bool empty() const {
        return size() == 0;
    }

    [[nodiscard]]
    constexpr bool contains(uintptr_t address) const {
        return (address >= start()) && (address < end());
    }

    [[nodiscard]]
    constexpr bool contains(const Chunk &other) const {
        return start() <= other.start() && end() >= other.end();
    }

    [[nodiscard]]
    constexpr bool overlaps(const Chunk &other) const {
        return start() < other.end() && other.start() < end();
    }

    [[nodiscard]]
    constexpr bool operator==(const Chunk &other) const {
        return start() == other.start() && size() == other.size();
    }

    [[nodiscard]]
    constexpr bool operator!=(const Chunk &other) const {
        return !(*this == other);
    }

  private:
    uintptr_t start_;
    size_t size_;
};

} // namespace DEPOS

#endif
