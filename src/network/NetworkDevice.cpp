#include <network/NetworkDevice.hpp>

namespace QUARK {

void *NetworkDevice::worker(void *argument) {
    auto *self = static_cast<NetworkDevice *>(argument);

    while (true) {
        self->semaphore_.p();

        if (!self->running_) break;

        NetworkBuffer *buffer = self->receive();

        if (!buffer) continue;

        self->notify(buffer);

        self->release(buffer);
    }
    return nullptr;
}

} // namespace QUARK
