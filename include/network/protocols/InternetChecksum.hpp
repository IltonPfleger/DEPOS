#pragma once

namespace DEPOS {

class InternetChecksum {
  public:
    InternetChecksum(const void *data, size_t length)
        : _result(0) {
        uint32_t sum            = 0;
        const uint16_t *pointer = reinterpret_cast<const uint16_t *>(data);

        for (; length > 1; length -= 2)
            sum += *pointer++;

        if (length > 0) sum += *reinterpret_cast<const uint8_t *>(pointer);

        while (sum >> 16)
            sum = (sum & 0xFFFF) + (sum >> 16);

        _result = static_cast<uint16_t>(~sum);
    }

    operator uint16_t() const { return _result; }

  private:
    uint16_t _result;
};

} // namespace DEPOS
