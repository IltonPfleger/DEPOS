#ifndef __MEMORY_CHUNK_HEADER__
#define __MEMORY_CHUNK_HEADER__

#include <types.hpp>

namespace DEPOS {

class Chunk {
  public:
    constexpr Chunk()
        : m_start(0),
          m_size(0) {}

    constexpr Chunk(uintptr_t start, size_t size)
        : m_start(start),
          m_size(size) {}

    constexpr Chunk(void *start, size_t size)
        : m_start(reinterpret_cast<uintptr_t>(start)),
          m_size(size) {}

    [[nodiscard]] constexpr uintptr_t start() const { return m_start; }
    [[nodiscard]] constexpr size_t size() const { return m_size; }
    [[nodiscard]] constexpr uintptr_t end() const { return start() + size(); }
    [[nodiscard]] unsigned char *data() const { return reinterpret_cast<unsigned char *>(start()); }

    [[nodiscard]] constexpr bool empty() const { return m_size == 0; }

    [[nodiscard]] constexpr bool contains(uintptr_t addr) const { return (addr >= m_start) && (addr < end()); }

    [[nodiscard]] constexpr bool overlaps(const Chunk &other) const {
        return m_start < other.end() && other.start() < end();
    }

    constexpr bool operator==(const Chunk &other) const { return m_start == other.m_start && m_size == other.m_size; }

    constexpr bool operator!=(const Chunk &other) const { return !(*this == other); }

  private:
    const uintptr_t m_start;
    const size_t m_size;
};

} // namespace DEPOS

#endif
