#pragma once

#include <network/protocols/IPv4.hpp>

namespace DEPOS {

class UDP {
    using Handler = IPv4;

    enum { Protocol = 17 };

    struct Header {
        uint16_t source;
        uint16_t destination;
        uint16_t length;
        uint16_t checksum;
    } __attribute__((packed));

  public:
    UDP(Handler *handler, uint16_t port)
        : _handler(handler),
          _port(port) {}

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

        auto *header        = reinterpret_cast<Header *>(buffer->data());
        header->source      = CPU::htobe16(_port);
        header->destination = CPU::htobe16(port);
        header->length      = CPU::htobe16(buffer->length());
        header->checksum    = 0;

        return _handler->send(address, Protocol, buffer);
    }

    void free(NetworkBuffer *buffer) { _handler->free(buffer); }

  private:
    Handler *_handler;
    uint16_t _port;
};

//
// struct Datagram {
//     auto header() const { return &m_header; }
//     uint16_t length() const { return CPU::be16toh(m_header.m_length) - sizeof(Header); }
//     const uint8_t *data() const { return reinterpret_cast<const uint8_t *>(&m_header + 1); }
//     uint8_t *data() { return reinterpret_cast<uint8_t *>(&m_header + 1); }
//
//   private:
//     Header m_header;
// };
//
// template <typename Network>
// class Channel : private Observer<const typename Network::Buffer *>, public Observed<const Datagram *> {
//
//   public:
//     typedef Datagram Buffer;
//     typedef DEPOS::Observer<const Datagram *> Observer;
//
//     Channel(uint16_t port = 0)
//         : m_network(Network::instance()),
//           m_port(port) {
//         m_network->attach(this);
//     }
//
//     ~Channel() { m_network->detach(this); }
//
//     NetworkBuffer *alloc(size_t size) { return m_network->alloc(size + sizeof(Header)); }
//     void free(NetworkBuffer *buffer) { return m_network->free(buffer); }
//
//     void send(Network::Address address, uint16_t port, NetworkBuffer *buffer) {
//         buffer->rewind(sizeof(Header));
//         buffer->length(buffer->length() + sizeof(Header));
//
//         auto *header          = reinterpret_cast<Header *>(buffer->data());
//         header->m_source      = CPU::htobe16(m_port);
//         header->m_destination = CPU::htobe16(port);
//         header->m_length      = CPU::htobe16(buffer->length());
//         header->m_checksum    = 0;
//
//         m_network->send(address, Protocol, buffer);
//     }
//
//     // void send(Network::Address address, uint16_t port, unsigned char *data, size_t length) {
//     //     auto *l2     = reinterpret_cast<Ethernet::Header *>(data);
//     //     auto *l3     = reinterpret_cast<Network::Header *>(l2 + 1);
//     //     auto *header = reinterpret_cast<Header *>(l3 + 1);
//
//   private:
//     void update(const Network::Buffer *packet) {
//         const auto *datagram = reinterpret_cast<const Datagram *>(packet->data());
//
//         if (packet->header()->m_protocol != Protocol) return;
//
//         if (!m_port || CPU::be16toh(datagram->header()->m_destination) == m_port) {
//             notify(datagram);
//         }
//     }
//
//   private:
//     Network *m_network;
//     uint16_t m_port;
// };

} // namespace DEPOS
