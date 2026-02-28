#pragma once

namespace DEPOS {

class Checksum {
  public:
    static uint16_t calculate(const void *data, size_t length) {
        uint32_t sum            = 0;
        const uint16_t *pointer = reinterpret_cast<const uint16_t *>(data);

        for (; length > 1; length -= 2)
            sum += *pointer++;

        if (length > 0) sum += *reinterpret_cast<const uint8_t *>(pointer);

        while (sum >> 16)
            sum = (sum & 0xFFFF) + (sum >> 16);

        return static_cast<uint16_t>(~sum);
    }
};

} // namespace DEPOS
