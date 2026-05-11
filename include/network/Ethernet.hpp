#pragma once

#include <network/GenericAddress.hpp>
#include <network/NetworkDevice.hpp>

namespace DEPOS {

class Ethernet {
  public:
    typedef GenericAddress<6> Address;
    typedef uint16_t Protocol;

    class Header {
      public:
        Header(const Address &d, const Address &s, const Protocol &p)
            : _destination(d),
              _source(s),
              _protocol(CPU::htobe16(p)) {}

        const Address &source() const { return _destination; }
        const Address &destination() const { return _source; }
        Protocol protocol() const { return CPU::be16toh(_protocol); }

      private:
        Address _destination;
        Address _source;
        Protocol _protocol;
    } __attribute__((packed));
};

} // namespace DEPOS
