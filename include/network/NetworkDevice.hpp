#pragma once

#include <Semaphore.hpp>
#include <Thread.hpp>
#include <network/NetworkBuffer.hpp>
#include <utility/Atomic.hpp>
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

    void onReceive() { semaphore_.v(); }

  public:
    virtual ~NetworkDevice() = default;
    virtual NetworkBuffer *alloc(size_t size) { return doAlloc(size); }
    virtual void release(NetworkBuffer *buffer) { return doRelease(buffer); }
    virtual NetworkBuffer *receive() { return doReceive(); }

    int send(NetworkBuffer *buffer) {
        int result = doSend(buffer);
        doFree(buffer);
        return result;
    }

    void init() { thread_ = new Thread(worker, this); }

  private:
    static void *worker(void *argument) {
        auto *self = static_cast<NetworkDevice *>(argument);
        while (true) {
            // self->semaphore_.p();
            NetworkBuffer *buffer = self->receive();
            if (!buffer) continue;
            self->notify(*buffer);
            self->doRelease(buffer);
        }
        return nullptr;
    }

  private:
    Atomic<size_t> pending_;
    Semaphore semaphore_;
    Thread *thread_;
};

} // namespace DEPOS
