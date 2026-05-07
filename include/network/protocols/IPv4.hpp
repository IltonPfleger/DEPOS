#pragma once

#include <ConditionalVariable.hpp>
#include <Meta.hpp>
#include <machine/Machine.hpp>
#include <network/GenericAddress.hpp>
#include <network/NetworkDevice.hpp>
#include <network/NetworkProtocolIdentifier.hpp>
#include <network/ethernet/Checksum.hpp>
#include <network/protocols/ARP.hpp>

namespace DEPOS {

struct NetworkLayerBuffer {
    NetworkAddress source;
    NetworkAddress destination;
    uint8_t protocol;
    NetworkBuffer *buffer;
};

class IPv4 : NetworkDevice::Observer {
  public:
    enum : uint16_t { Protocol = 0x0800 };
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
        Address to;

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
            to             = d;
            checksum       = 0;
            checksum       = Checksum::calculate(this, sizeof(Header));
        }

        uint8_t length() const { return (version & 0x0F) * 4; }

    } __attribute__((packed));

    IPv4(NetworkAddressableDevice *device, ARP *arp)
        : _device(device),
          _arp(arp) {
        _arp->bind(address());
        _device->attach(this);
    }

    Address address() const { return Address(192, 168, 1, 1); }

    ~IPv4() { _device->detach(this); }

    NetworkBuffer *alloc(size_t length) {
        size_t total          = length + sizeof(Header);
        NetworkBuffer *buffer = _device->alloc(total);
        buffer->advance(sizeof(Header));
        buffer->length(total);
        return buffer;
    }

    void free(NetworkBuffer *buffer) { _device->free(buffer); }

  public:
    void update(const NetworkBuffer *buffer) {
        (void)buffer;
        //_lock.acquire();
        //_device->clone(buffer, _receivers.count());
    }

    int send(const NetworkAddress &pa, uint8_t protocol, NetworkBuffer *buffer) {
        buffer->rewind(sizeof(Header));

        new (buffer->data()) Header(pa, this->address(), protocol, buffer->length());

        buffer->length(buffer->length() + sizeof(Header));

        if (Address(pa) == Address::broadcast()) {
            return _device->broadcast(NetworkProtocolIdentifier::IPv4(), buffer);
        } else {
            return _device->send(_arp->resolve(pa), NetworkProtocolIdentifier::IPv4(), buffer);
        }
    }

    // NetworkLayerBuffer receive() {
    //     _lock.acquire();
    //     NetworkBuffer *buffer = reinterpret_cast<NetworkBuffer *>(_receivers.wait(&_lock));
    // }

  private:
    NetworkAddressableDevice *_device;
    // ConditionalVariable _receivers;
    Spin _lock;
    ARP *_arp;
};

// class Packet {
//   public:
//     const Header *header() const { return reinterpret_cast<const Header *>(&header); }
//
//     uint16_t length() const { return CPU::be16toh(header()->length) - header()->length(); }
//
//     uint8_t *data() { return reinterpret_cast<uint8_t *>(this) + header()->length(); }
//
//     const uint8_t *data() const { return reinterpret_cast<const uint8_t *>(this) + header()->length(); }
//
//   private:
//     Header header;
// };
//
// template <typename Device> class Network : public Device::Observer, public Observed<const Packet *> {
//     using ARP = DEPOS::ARP<Device, Network>;
//     //    using Family   = typename Device::Family;
//     using HA       = typename Device::Address;
//     using MyTraits = Device::MyTraits;
//
//   public:
//     enum { Protocol = IPv4::Protocol };
//     using Buffer  = IPv4::Packet;
//     using Header  = IPv4::Header;
//     using Address = Address;
//     using PA      = Address;
//
//
//     void send(PA pa, uint8_t protocol, NetworkBuffer *buffer) {
//         HA ha;
//         if (is_broadcast(pa)) {
//             ha = HA::broadcast();
//         } else {
//             ha = arp->resolve(pa);
//         }
//         send(ha, pa, protocol, buffer);
//     }
//
//   protected:
//     Network() {
//         device = Device::instance();
//         arp    = new ARP(device, this);
//         device->attach(this);
//     }
//
//     ~Network() { device->detach(this); }
//
//     void update(const NetworkBuffer *buffer, const NetworkProtocolIdentifier &&protocol) override {
//         auto *packet = buffer->data<Packet *>();
//
//         if (protocol == NetworkProtocolIdentifier::IPv4()) {
//             if (accepts(packet)) {
//                 this->notify(packet);
//             }
//         }
//     }
//
//   private:
//     bool is_broadcast(PA pa) {
//         if (pa == PA::broadcast()) return true;
//         // if ((PA(MyTraits::Netmask) | pa) == PA::broadcast()) return true;
//         return false;
//     }
//
//     bool accepts(const Packet *) const { return true; }
//
//     void send(HA ha, PA pa, uint8_t protocol, NetworkBuffer *buffer) {
//         buffer->rewind(sizeof(Header));
//
//         new (buffer->data()) Header(pa, this->address(), protocol, buffer->length());
//
//         buffer->length(buffer->length() + sizeof(Header);
//
//         device->send(ha, NetworkProtocolIdentifier::IPv4(), buffer);
//     }
//
//   public:
//     static auto instance() {
//         if (!s_instance) s_instance = new Network();
//         return s_instance;
//     }
//
//     NetworkBuffer *alloc(size_t size) { return device->alloc(size + sizeof(Header)); }
//     void free(NetworkBuffer *buffer) { device->free(buffer); }
//
//   private:
//     static inline Network *s_instance = nullptr;
//     Device *device;
//     ARP *arp;
// };

} // namespace DEPOS
