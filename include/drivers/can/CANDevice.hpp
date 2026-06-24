#ifndef __QUARK_CAN_DEVICE__
#define __QUARK_CAN_DEVICE__

#include <libraries/libc/string.h>
#include <network/CAN.hpp>
#include <network/NetworkDevice.hpp>

namespace QUARK {

class CANDevice : public NetworkDevice {
  public:
    int send(uint32_t id, unsigned char *data, size_t length = 8, bool ide = false, bool rtr = false) {
        NetworkBuffer *raw  = this->alloc(length);
        CAN::Buffer *buffer = static_cast<CAN::Buffer *>(raw);
        new (buffer) CAN::Buffer(id, length, ide, rtr);
        memcpy(buffer->data(), data, length);
        return this->doSend(buffer);
    }
};

} // namespace QUARK

#endif
