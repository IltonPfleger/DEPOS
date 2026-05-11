#ifndef __DEPOS_NETWORK_LINK_LAYER__
#define __DEPOS_NETWORK_LINK_LAYER__

#include <network/NetworkAddress.hpp>
#include <network/NetworkBuffer.hpp>
#include <utility/Observer.hpp>

namespace DEPOS {

class NetworkLinkLayer : public Observed<NetworkBuffer> {
  public:
    virtual ~NetworkLinkLayer()                               = default;
    virtual void bind(const NetworkAddress &)                 = 0;
    virtual NetworkBuffer *alloc(size_t)                      = 0;
    virtual int send(const NetworkAddress &, NetworkBuffer *) = 0;
};

} // namespace DEPOS

#endif
