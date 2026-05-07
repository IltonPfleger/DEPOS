#pragma once

#include <network/protocols/IPv4.hpp>

namespace DEPOS {

class UDP : public Observer<NetworkBuffer, const NetworkAddress &, const NetworkAddress &, uint8_t>,
            public Observed<NetworkBuffer, uint16_t, uint16_t> {

    using Handler = IPv4;

    enum { Protocol = 17 };

    struct Header {
        uint16_t source;
        uint16_t destination;
        uint16_t length;
        uint16_t checksum;
    } __attribute__((packed));

  public:
    UDP(Handler *handler, uint16_t port = 0)
        : _handler(handler),
          _port(port) {
        _handler->attach(this);
    }

    NetworkBuffer *alloc(size_t length) {
        size_t total          = length + sizeof(Header);
        NetworkBuffer *buffer = _handler->alloc(total);
        buffer->advance(sizeof(Header));
        buffer->length(length);
        return buffer;
    }

    int send(const NetworkAddress &address, uint16_t port, NetworkBuffer *buffer) {
        buffer->rewind(sizeof(Header));
        buffer->length(buffer->length() + sizeof(Header));
        Header *header      = buffer->data<Header *>();
        header->source      = CPU::htobe16(_port);
        header->destination = CPU::htobe16(port);
        header->length      = CPU::htobe16(buffer->length());
        header->checksum    = 0;
        return _handler->send(address, Protocol, buffer);
    }

    void update(NetworkBuffer buffer, const NetworkAddress &, const NetworkAddress &, uint8_t protocol) {
        if (protocol != Protocol) return;
        Header *header = buffer.data<Header *>();
        buffer.advance(sizeof(Header));
        buffer.length(CPU::htobe16(header->length) - sizeof(Header));
        notify(buffer, CPU::be16toh(header->destination), CPU::be16toh(header->source));
    }

    void free(NetworkBuffer *buffer) { _handler->free(buffer); }

  private:
    Handler *_handler;
    uint16_t _port;
};

} // namespace DEPOS
