#pragma once

#include <network/ethernet/ip/IPv4.hpp>

namespace DEPOS {

class Multicast {
    typedef IPv4::IP IP;
    typedef Ethernet::MAC MAC;

    struct IGMPv2 {
        uint8_t m_type;
        uint8_t m_time;
        uint16_t m_checksum;
        IP m_group;

        IGMPv2(uint8_t type, IP group)
            : m_type(type),
              m_time(0),
              m_checksum(0),
              m_group(group) {}
    } __attribute__((packed));

    static constexpr IP All = IP(224, 0, 0, 2);

    static MAC convert_multicast_group_ip_to_mac(const IP &group) {
        return MAC(0x01, 0x00, 0x5E, group[1] & 0x7F, group[2], group[3]);
    }

  public:
    template <typename Driver> static void join(IPv4::Connection<Driver> *socket, IP group) {
        unsigned char buffer[1024];
        new (buffer + sizeof(Ethernet::Header) + sizeof(IPv4::Header)) IGMPv2(0x16, group);
        MAC mac   = convert_multicast_group_ip_to_mac(group);
        MAC mymac = Driver::instance()->mac();
        IP myip   = Driver::instance()->ip();
        socket->send(mac, group, mymac, myip, IPv4::IGMP, buffer, sizeof(IGMPv2));
    }

    // template <typename Driver> static bool leave(IP group) {
    //     IPv4::Connection<Driver> socket;
    //     unsigned char buffer[sizeof(IGMPv2) + sizeof(IPv4::Header) + sizeof(Ethernet::Header)];

    //    new (buffer) IGMPv2(0x17, group);
    //    // IGMPv2 *report = new (buffer) IGMPv2(0x17, group);
    //    //  report->m_checksum = IPv4::checksum(report, sizeof(IGMPv2));

    //    MAC mac   = convert_multicast_group_ip_to_mac(All);
    //    MAC mymac = Driver::instance()->mac();
    //    IP myip   = Driver::instance()->ip();

    //    return socket.send(mac, All, mymac, myip, IPv4::IGMP, buffer, sizeof(IGMPv2));
    //}
};

} // namespace DEPOS
