#pragma once

#include <network/Ethernet.hpp>
#include <network/NetworkDevice.hpp>

namespace DEPOS {

class EthernetDevice : public NetworkDevice {
  public:
    using Address = Ethernet::Address;
    using Header  = Ethernet::Header;
    using NetworkDevice::send;

    virtual ~EthernetDevice() {}

    virtual NetworkAddress address() const       = 0;
    virtual void address(const NetworkAddress &) = 0;

    NetworkBuffer *alloc(size_t length) override {
        NetworkBuffer *buffer = doAlloc(length + sizeof(Header));
        buffer->advance(sizeof(Header));
        return buffer;
    }

    NetworkBuffer *receive() override {
        NetworkBuffer *buffer = doReceive();
        if (buffer) buffer->advance(sizeof(Header));
        return buffer;
    }

    int send(const NetworkAddress &destination, uint16_t protocol, NetworkBuffer *buffer) {
        buffer->rewind(sizeof(Header));
        new (buffer->data()) Header(destination, address(), protocol);
        return send(buffer);
    };
};

} // namespace DEPOS
