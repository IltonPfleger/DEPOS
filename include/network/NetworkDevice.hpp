#pragma once

#include <Semaphore.hpp>
#include <Thread.hpp>
#include <network/NetworkBuffer.hpp>
#include <utility/Observer.hpp>

namespace DEPOS {

class NetworkDevice : public Observed<NetworkBuffer> {
  public:
    using Observer = DEPOS::Observer<NetworkBuffer>;

  protected:
    virtual NetworkBuffer *doAlloc(size_t)  = 0;
    virtual int doSend(NetworkBuffer *)     = 0;
    virtual void doFree(NetworkBuffer *)    = 0;
    virtual NetworkBuffer *doReceive()      = 0;
    virtual void doRelease(NetworkBuffer *) = 0;

  public:
    virtual ~NetworkDevice() = default;
    virtual NetworkBuffer *alloc(size_t size) { return doAlloc(size); }
    virtual NetworkBuffer *receive() { return doReceive(); }

    int send(NetworkBuffer *buffer) {
        int result = doSend(buffer);
        doFree(buffer);
        return result;
    }

    void init() { thread_ = new Thread(worker, this); }

    void onReceive() { pending_.v(); }

  private:
    static void *worker(void *argument) {
        auto *self = static_cast<NetworkDevice *>(argument);
        while (true) {
            self->pending_.p();
            NetworkBuffer *buffer = self->receive();
            if (!buffer) continue;
            self->notify(*buffer);
            self->doRelease(buffer);
        }
        return nullptr;
    }

  private:
    Semaphore pending_;
    Thread *thread_;
};

} // namespace DEPOS
