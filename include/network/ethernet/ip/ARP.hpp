#pragma once

#include <Spin.hpp>
#include <network/NIC.hpp>
#include <network/ethernet/Ethernet.hpp>
#include <utils/Observer.hpp>
#include <utils/lists/Hash.hpp>

namespace DEPOS {

template <typename NIC, typename Network> class ARP : public NIC::Observer {
  private:
    enum { Protocol = 0x0806 };
    enum class Opcode : uint16_t { REQUEST = 1, REPLY = 2 };

    struct Header {
        uint16_t htype{CPU::htobe16(1)};
        uint16_t ptype{CPU::htobe16(Network::Protocol)};
        uint8_t hlen{6};
        uint8_t plen{sizeof(typename Network::Address)};
        uint16_t operation{0};

        Ethernet::Address sha;
        Network::Address spa;
        Ethernet::Address tha;
        Network::Address tpa;

    } __attribute__((packed));

    struct Entry {
        Ethernet::Address mac{};
        Network::Address ip{};
        Thread::Queue waiting{};
        bool valid{false};
        uint32_t pending{0};

        void wakeup() {
            while (pending > 0) {
                pending--;
                Thread::wakeup(&waiting);
            }
        }
    };

    struct Hasher {
        size_t operator()(const typename Network::Address &addr) const { return addr[3]; }
    };

    using Table = Hash<typename Network::Address, Entry, 256, Hasher>;

  public:
    ARP(NIC *nic, Network *network)
        : m_nic(nic),
          m_network(network) {
        m_nic->attach(this);
    }

    Ethernet::Address resolve(const Network::Address &ip) {
        Ethernet::Address mac;
        while (true) {
            m_spin.acquire();
            auto &entry = m_table[ip];

            if (entry.valid && entry.ip == ip) {
                mac = entry.mac;
                m_spin.release();
                return mac;
            }

            entry.pending++;
            send(Ethernet::Broadcast, ip, Opcode::REQUEST);
            Thread::sleep(&entry.waiting, &m_spin);
        }
    }

    void update(const NIC::Buffer *buffer) override {
        auto *eth = reinterpret_cast<const Ethernet::Header *>(buffer->data());
        if (buffer->length() < sizeof(Ethernet::Header) + sizeof(Header) || eth->protocol() != Protocol) return;

        const auto *arp = reinterpret_cast<const Header *>(eth + 1);
        auto op         = static_cast<Opcode>(CPU::be16toh(arp->operation));

        if (op == Opcode::REQUEST)
            request(*arp);
        else if (op == Opcode::REPLY)
            reply(*arp);
    }

  private:
    void request(const Header &arp) {
        if (arp.tpa == m_network->address()) send(arp.sha, arp.spa, Opcode::REPLY);
    }

    void reply(const Header &arp) {
        m_spin.acquire();
        auto &entry = m_table[arp.spa];
        entry.ip    = arp.spa;
        entry.mac   = arp.sha;
        entry.valid = true;
        entry.wakeup();
        m_spin.release();
    }

    void send(const Ethernet::Address &tha, const Network::Address &tpa, Opcode op) {
        alignas(64) unsigned char buffer[sizeof(Ethernet::Header) + sizeof(Header)];
        auto *eth      = new (buffer) Ethernet::Header(tha, m_nic->address(), Protocol);
        auto *arp      = new (eth + 1) Header();
        arp->operation = CPU::htobe16(static_cast<uint16_t>(op));
        arp->sha       = m_nic->address();
        arp->spa       = m_network->address();
        arp->tha       = (op == Opcode::REQUEST) ? Ethernet::Address("00:00:00:00:00:00") : tha;
        arp->tpa       = tpa;
        m_nic->send(buffer, sizeof(buffer));
    }

  private:
    NIC *m_nic;
    Network *m_network;
    Table m_table;
    Spin m_spin;
};

} // namespace DEPOS
