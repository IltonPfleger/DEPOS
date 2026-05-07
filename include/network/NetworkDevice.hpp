#pragma once

#include <Thread.hpp>
#include <network/NetworkBuffer.hpp>
#include <utility/Observer.hpp>

namespace DEPOS {

class NetworkDevice : public Observed<NetworkBuffer> {
  protected:
    virtual NetworkBuffer *doAlloc(size_t)  = 0;
    virtual int doSend(NetworkBuffer *)     = 0;
    virtual void doFree(NetworkBuffer *)    = 0;
    virtual NetworkBuffer *doReceive()      = 0;
    virtual void doRelease(NetworkBuffer *) = 0;

  public:
    virtual ~NetworkDevice() = default;
    NetworkBuffer *alloc(size_t size) { return doAlloc(size); }
    int send(NetworkBuffer *buffer) { return doSend(buffer); }
    void free(NetworkBuffer *buffer) { return doFree(buffer); }

    void init() { _thread = new Thread(worker, this); }

    void release(NetworkBuffer *buffer) {
        if (CPU::Atomic::fdec(buffer->internal()->references) == 1) doRelease(buffer);
    }

    // void retain(const NetworkBuffer *buffer) { CPU::Atomic::finc(buffer->internal()->references); }

  private:
    static void *worker(void *argument) {
        auto *self = static_cast<NetworkDevice *>(argument);
        while (true) {
            NetworkBuffer *buffer = self->doReceive();
            if (!buffer) continue;
            buffer->internal()->references = 1;
            self->notify(*buffer);
            self->release(buffer);
        }
        return nullptr;
    }

  private:
    Thread *_thread;
};

} // namespace DEPOS
