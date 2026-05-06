#pragma once

#include <Meta.hpp>
#include <machine/Machine.hpp>
#include <network/GenericAddress.hpp>
#include <network/NetworkDevice.hpp>
#include <network/NetworkProtocolIdentifier.hpp>
#include <network/ethernet/Checksum.hpp>
#include <network/protocols/ARP.hpp>

namespace DEPOS {

class IPv4 {
  public:
    enum : uint16_t { Protocol = 0x0800 };
    enum : uint8_t { DefaultTTL = 64, VersionIHL = 0x45 };

    using Address = GenericAddress<4>;

    struct Header {
        uint8_t m_version;
        uint8_t m_tos;
        uint16_t m_length;
        uint16_t m_identification;
        uint16_t m_fragment;
        uint8_t m_ttl;
        uint8_t m_protocol;
        uint16_t m_checksum;
        Address m_source;
        Address m_to;

        Header(Address destination,
               Address source,
               uint8_t protocol,
               uint16_t length,
               uint8_t tos             = 0,
               uint16_t identification = 0,
               uint16_t fragment       = 0,
               uint8_t ttl             = DefaultTTL) {
            m_version        = VersionIHL;
            m_tos            = tos;
            m_length         = CPU::htobe16(sizeof(Header) + length);
            m_identification = CPU::htobe16(identification);
            m_fragment       = CPU::htobe16(fragment);
            m_ttl            = ttl;
            m_protocol       = protocol;
            m_source         = source;
            m_to             = destination;
            m_checksum       = 0;
            m_checksum       = Checksum::calculate(this, sizeof(Header));
        }

        uint8_t length() const { return (m_version & 0x0F) * 4; }

    } __attribute__((packed));
};

// class Packet {
//   public:
//     const Header *header() const { return reinterpret_cast<const Header *>(&m_header); }
//
//     uint16_t length() const { return CPU::be16toh(header()->m_length) - header()->length(); }
//
//     uint8_t *data() { return reinterpret_cast<uint8_t *>(this) + header()->length(); }
//
//     const uint8_t *data() const { return reinterpret_cast<const uint8_t *>(this) + header()->length(); }
//
//   private:
//     Header m_header;
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
//     auto address() const { return PA(MyTraits::IP); }
//
//     void send(PA pa, uint8_t protocol, NetworkBuffer *buffer) {
//         HA ha;
//         if (is_broadcast(pa)) {
//             ha = HA::broadcast();
//         } else {
//             ha = m_arp->resolve(pa);
//         }
//         send(ha, pa, protocol, buffer);
//     }
//
//   protected:
//     Network() {
//         m_device = Device::instance();
//         m_arp    = new ARP(m_device, this);
//         m_device->attach(this);
//     }
//
//     ~Network() { m_device->detach(this); }
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
//         m_device->send(ha, NetworkProtocolIdentifier::IPv4(), buffer);
//     }
//
//   public:
//     static auto instance() {
//         if (!s_instance) s_instance = new Network();
//         return s_instance;
//     }
//
//     NetworkBuffer *alloc(size_t size) { return m_device->alloc(size + sizeof(Header)); }
//     void free(NetworkBuffer *buffer) { m_device->free(buffer); }
//
//   private:
//     static inline Network *s_instance = nullptr;
//     Device *m_device;
//     ARP *m_arp;
// };

} // namespace DEPOS
