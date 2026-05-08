#ifndef __DEPOS_IPV4_TO_ETHERNET_LINK_LAYER__
#define __DEPOS_IPV4_TO_ETHERNET_LINK_LAYER__

#include <network/NetworkAddress.hpp>
#include <network/NetworkAddressableDevice.hpp>
#include <network/NetworkBuffer.hpp>
#include <network/NetworkLinkLayer.hpp>
#include <network/protocols/ARP.hpp>
#include <network/protocols/IPv4.hpp>

namespace DEPOS {

class LinkIPv4ToEthernet : public NetworkLinkLayer {
    using MAC    = GenericAddress<6>;
    using IP     = GenericAddress<4>;
    using Router = ARP<Ethernet, IPv4>;

  public:
    virtual ~LinkIPv4ToEthernet() = default;

    LinkIPv4ToEthernet(NetworkAddressableDevice &device)
        : device_(device),
          router_(Router(device_)) {}

    virtual void bind(const NetworkAddress &address) override { router_.bind(address); }

    virtual int send(const NetworkAddress &address, NetworkBuffer *buffer) override {
        if (IP(address) == IPv4Broadcast) return device_.send(EthernetBroadcast, Ethertype, buffer);

        unsigned char data[16];
        if (router_.resolve(address, Span(data))) {
            NetworkAddress ha(data, device_.address().length());
            return device_.send(ha, Ethertype, buffer);
        }

        return 0;
    }

  private:
    static constexpr Ethernet::Protocol Ethertype = IPv4::ProtocolValue;
    static constexpr MAC EthernetBroadcast        = {255, 255, 255, 255, 255, 255};
    static constexpr IP IPv4Broadcast             = {255, 255, 255, 255};

  private:
    NetworkAddressableDevice &device_;
    Router router_;
};

} // namespace DEPOS

#endif
