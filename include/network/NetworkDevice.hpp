#pragma once

#include <Thread.hpp>
#include <network/NetworkBuffer.hpp>

namespace DEPOS {

template <typename F> class NetworkDevice : public Observed<const NetworkBuffer *> {
  public:
    using Buffer   = NetworkBuffer;
    using Observer = DEPOS::Observer<const Buffer *>;
    using Observed = DEPOS::Observed<const Buffer *>;
    using Family   = F;
    using Address  = Family::Address;

    virtual ~NetworkDevice()          = default;
    virtual int send(Buffer *)        = 0;
    virtual void free(Buffer *b)      = 0;
    virtual Buffer *alloc()           = 0;
    virtual Buffer *receive()         = 0;
    virtual Family::Address address() = 0;

  protected:
    void init() { new Thread(worker, this); }

  private:
    static void *worker(void *argument) {
        auto *self = static_cast<NetworkDevice *>(argument);
        while (true) {
            auto *buffer = self->receive();
            if (!buffer) continue;
            self->notify(buffer);
            self->free(buffer);
        }
        return nullptr;
    }
};

} // namespace DEPOS
