#pragma once

#include <network/ethernet/ip/IGMP.hpp>
#include <network/ethernet/ip/IPv4.hpp>

namespace DEPOS {

class Multicast {

    static Ethernet::Address ip_to_mac(const IPv4::Address &ip) {
        return {0x01, 0x00, 0x5E, ip[1] & 0x7F, ip[2], ip[3]};
    }

  public:
    template <typename Device> static void join(IPv4::Address group) {
        unsigned char buffer[1024];

        new (buffer + sizeof(Ethernet::Header) + sizeof(IPv4::Header))
            IGMP::Header(IGMP::Report, group);

        auto dmac = mac(group);
        IPv4::Network<NIC<Device>>::instance()->send(dmac, group, IGMP::Protocol, buffer,
                                                     sizeof(IGMP::Header));
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
