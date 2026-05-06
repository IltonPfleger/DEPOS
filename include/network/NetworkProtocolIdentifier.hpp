#ifndef __DEPOS_NETWORK_PROTOCOL_IDENTIFIER_HEADER__
#define __DEPOS_NETWORK_PROTOCOL_IDENTIFIER_HEADER__

#include <types.hpp>

namespace DEPOS {

class NetworkProtocolIdentifier {
  public:
    constexpr NetworkProtocolIdentifier(uint16_t v)
        : _value(v) {}

    constexpr operator uint16_t() const { return _value; }

    constexpr bool operator==(const NetworkProtocolIdentifier &other) const { return _value == other._value; }
    constexpr bool operator!=(const NetworkProtocolIdentifier &other) const { return _value != other._value; }

    static constexpr NetworkProtocolIdentifier UNKNOWN() { return NetworkProtocolIdentifier(0x0000); }
    static constexpr NetworkProtocolIdentifier ARP() { return NetworkProtocolIdentifier(0x0806); }
    static constexpr NetworkProtocolIdentifier VLAN() { return NetworkProtocolIdentifier(0x8100); }
    static constexpr NetworkProtocolIdentifier ICMP() { return NetworkProtocolIdentifier(0x0001); }
    static constexpr NetworkProtocolIdentifier IPv4() { return NetworkProtocolIdentifier(0x0800); }
    static constexpr NetworkProtocolIdentifier IPv6() { return NetworkProtocolIdentifier(0x86DD); }
    static constexpr NetworkProtocolIdentifier ICMPv6() { return NetworkProtocolIdentifier(0x003A); }
    static constexpr NetworkProtocolIdentifier CAN() { return NetworkProtocolIdentifier(0x000C); }

  private:
    uint16_t _value;
};

} // namespace DEPOS

#endif
