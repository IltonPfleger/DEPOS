#ifndef __DEPOS_NETWORK_ADDRESSABLE_DEVICE__
#define __DEPOS_NETWORK_ADDRESSABLE_DEVICE__

#include <network/NetworkAddress.hpp>
#include <network/NetworkDevice.hpp>
#include <network/NetworkProtocolIdentifier.hpp>

namespace DEPOS {

class NetworkAddressableDevice : public NetworkDevice {
  public:
    using NetworkDevice::send;

    virtual ~NetworkAddressableDevice() {}

    virtual NetworkAddress address() const       = 0;
    virtual void address(const NetworkAddress &) = 0;

    virtual int broadcast(const NetworkProtocolIdentifier &, NetworkBuffer *)                    = 0;
    virtual int send(const NetworkAddress &, const NetworkProtocolIdentifier &, NetworkBuffer *) = 0;
};

} // namespace DEPOS

#endif
