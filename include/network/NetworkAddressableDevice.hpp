#ifndef __DEPOS_NETWORK_ADDRESSABLE_DEVICE__
#define __DEPOS_NETWORK_ADDRESSABLE_DEVICE__

#include <network/NetworkAddress.hpp>
#include <network/NetworkDevice.hpp>

namespace DEPOS {

class NetworkAddressableDevice : public NetworkDevice {
  public:
    using NetworkDevice::send;

    virtual ~NetworkAddressableDevice() {}

    virtual NetworkAddress address() const                              = 0;
    virtual void address(const NetworkAddress &)                        = 0;
    virtual int send(const NetworkAddress &, uint16_t, NetworkBuffer *) = 0;
};

} // namespace DEPOS

#endif
