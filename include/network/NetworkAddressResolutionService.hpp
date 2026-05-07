#ifndef __DEPOS_NETWORK_ADDRESS_RESOLUTION_SERVICE_HEADER__
#define __DEPOS_NETWORK_ADDRESS_RESOLUTION_SERVICE_HEADER__

#include <network/NetworkAddress.hpp>
#include <network/NetworkBuffer.hpp>

namespace DEPOS {

class NetworkAddressResolutionService {
  public:
    virtual ~NetworkAddressResolutionService()             = default;
    virtual NetworkAddress resolve(const NetworkAddress &) = 0;
};

} // namespace DEPOS

#endif
