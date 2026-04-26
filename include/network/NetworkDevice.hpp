#ifndef __NETWORK_DEVICE_HEADER__
#define __NETWORK_DEVICE_HEADER__

#include <kernel/Thread.hpp>
#include <network/NetworkAddress.hpp>
#include <network/NetworkBuffer.hpp>
#include <utils/Observer.hpp>

namespace DEPOS {

class NetworkDevice : Observed<const NetworkBuffer *> {
  public:
    using Observer = DEPOS::Observer<const NetworkBuffer *>;
    using Observed = DEPOS::Observed<const NetworkBuffer *>;

  public:
    virtual ~NetworkDevice() {}
    virtual int send(const NetworkBuffer *)  = 0;
    virtual void free(const NetworkBuffer *) = 0;
    virtual const NetworkBuffer *receive()   = 0;

  protected:
    void init() { new Thread(jump, this); }

  private:
    static void *jump(void *p) { return static_cast<NetworkDevice *>(p)->work(); }

    void *work() {
        while (true) {
            auto *buffer = receive();
            if (!buffer) continue;
            notify(buffer);
            free(buffer);
        }
        return nullptr;
    }
};

} // namespace DEPOS

#endif
