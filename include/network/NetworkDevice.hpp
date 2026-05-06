#pragma once

#include <Thread.hpp>
#include <network/NetworkBuffer.hpp>
#include <utils/Observer.hpp>

namespace DEPOS {

class NetworkDevice : public Observed<const NetworkBuffer *> {
  public:
    using Buffer   = NetworkBuffer;
    using Observer = DEPOS::Observer<const Buffer *>;

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

    void init() {
        _running = true;
        _thread  = new Thread(worker, this);
    }

  private:
    static void *worker(void *argument) {
        auto *self = static_cast<NetworkDevice *>(argument);
        while (self->_running) {
            NetworkBuffer *buffer = self->doReceive();
            if (!buffer) continue;
            self->notify(buffer);
            self->doRelease(buffer);
        }
        return nullptr;
    }

  private:
    Thread *_thread;
    volatile bool _running;
};

// template <typename F> class NetworkDevice : public Observed<const NetworkBuffer *> {
//   public:
//     using Buffer   = NetworkBuffer;
//     using Observer = DEPOS::Observer<const Buffer *>;
//     using Observed = DEPOS::Observed<const Buffer *>;
//     using Family   = F;
//     using Address  = Family::Address;
//
//     virtual ~NetworkDevice()          = default;
//     virtual int send(Buffer *)        = 0;
//     virtual void free(Buffer *b)      = 0;
//     virtual Buffer *alloc()           = 0;
//     virtual Buffer *receive()         = 0;
//     virtual Family::Address address() = 0;
//
//   protected:
//     void init() { new Thread(worker, this); }
//
//   private:
//     static void *worker(void *argument) {
//         auto *self = static_cast<NetworkDevice *>(argument);
//         while (true) {
//             auto *buffer = self->receive();
//             if (!buffer) continue;
//             self->notify(buffer);
//             self->free(buffer);
//         }
//         return nullptr;
//     }
// };

} // namespace DEPOS
