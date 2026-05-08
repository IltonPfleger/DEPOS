#pragma once

#include <Meta.hpp>
#include <machine/Machine.hpp>
#include <network/GenericAddress.hpp>
#include <network/NetworkAddressableDevice.hpp>
#include <network/NetworkLinkLayer.hpp>
#include <network/protocols/InternetChecksum.hpp>

namespace DEPOS {

struct NetworkLayerBuffer {
    NetworkBuffer buffer;
    NetworkAddress destination;
    NetworkAddress source;
    uint8_t protocol;
};

class IPv4 : public Observer<NetworkBuffer>,
             public Observed<NetworkBuffer, const NetworkAddress &, const NetworkAddress &, uint8_t> {
  public:
    enum : uint16_t { ProtocolValue = 0x0800 };
    enum : uint8_t { DefaultTTL = 64, VersionIHL = 0x45 };

    using Address = GenericAddress<4>;

    struct Header {
        uint8_t version;
        uint8_t tos;
        uint16_t tlength;
        uint16_t identification;
        uint16_t fragment;
        uint8_t ttl;
        uint8_t protocol;
        uint16_t checksum;
        Address source;
        Address destination;

        Header(Address d,
               Address s,
               uint8_t p,
               uint16_t l,
               uint8_t t    = 0,
               uint16_t id  = 0,
               uint16_t n   = 0,
               uint8_t time = DefaultTTL) {
            version        = VersionIHL;
            tos            = t;
            tlength        = CPU::htobe16(sizeof(Header) + l);
            identification = CPU::htobe16(id);
            fragment       = CPU::htobe16(n);
            ttl            = time;
            protocol       = p;
            source         = s;
            destination    = d;
            checksum       = 0;
            checksum       = InternetChecksum(this, sizeof(Header));
        }

        uint8_t length() const { return (version & 0x0F) * 4; }

    } __attribute__((packed));

    IPv4(const Address &address, NetworkAddressableDevice &device, NetworkLinkLayer &link)
        : device_(device),
          link_(link) {
        _address = address;
        link_.bind(_address);
        device_.attach(this);
    }

    ~IPv4() { device_.detach(this); }

    NetworkBuffer *alloc(size_t length) {
        size_t total          = length + sizeof(Header);
        NetworkBuffer *buffer = device_.alloc(total);
        buffer->advance(sizeof(Header));
        buffer->length(total);
        return buffer;
    }

  public:
    void update(NetworkBuffer buffer) {
        if (buffer.protocol() != ProtocolValue) return;
        Header *header = buffer.data<Header *>();
        buffer.advance(header->length());
        notify(buffer, header->destination, header->source, header->protocol);
    }

    int send(const NetworkAddress &pa, uint8_t protocol, NetworkBuffer *buffer) {
        buffer->rewind(sizeof(Header));

        new (buffer->data()) Header(pa, _address, protocol, buffer->length());

        buffer->length(buffer->length() + sizeof(Header));

        return link_.send(pa, buffer);
    }

  private:
    Address _address;
    NetworkAddressableDevice &device_;
    NetworkLinkLayer &link_;
};

} // namespace DEPOS
