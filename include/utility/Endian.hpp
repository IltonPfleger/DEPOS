#include <types.hpp>

namespace QUARK {

class Endian {
  public:
    template <typename T> static T swap(T x) {
        T result = 0;
        for (size_t i = 0; i < sizeof(T); ++i) {
            result |= ((x >> (i * 8)) & 0xFF) << ((sizeof(T) - 1 - i) * 8);
        }
        return result;
    }

    static uint8_t be2le8(uint8_t x) { return x; }
    static uint8_t le2be8(uint8_t x) { return x; }

    static uint16_t be2le16(uint16_t x) { return swap(x); }
    static uint16_t le2be16(uint16_t x) { return swap(x); }

    static uint32_t be2le32(uint32_t x) { return swap(x); }
    static uint32_t le2be32(uint32_t x) { return swap(x); }

    static uint64_t be2le64(uint64_t x) { return swap(x); }
    static uint64_t le2be64(uint64_t x) { return swap(x); }
};

} // namespace QUARK
