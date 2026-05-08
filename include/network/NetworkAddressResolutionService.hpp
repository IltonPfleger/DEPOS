#ifndef __DEPOS_NETWORK_ADDRESS_RESOLUTION_SERVICE_HEADER__
#define __DEPOS_NETWORK_ADDRESS_RESOLUTION_SERVICE_HEADER__

#include <network/NetworkAddress.hpp>
#include <network/NetworkBuffer.hpp>

namespace DEPOS {

class NetworkAddressResolutionService {
  public:
    virtual ~NetworkAddressResolutionService()                  = default;
    virtual void bind(const NetworkAddress &)                   = 0;
    virtual bool resolve(const NetworkAddress &, Span<uint8_t>) = 0;
};

} // namespace DEPOS

#endif
