#pragma once

#include <Spin.hpp>
#include <network/NIC.hpp>
#include <network/ethernet/Ethernet.hpp>
#include <utils/Observer.hpp>

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

    class Table {
      private:
        struct Entry {
            Ethernet::Address mac{};
            Network::Address ip{};
            Thread::Queue waiting{};
            bool valid{false};
            uint32_t pending{0};
        };

        static constexpr size_t TABLE_SIZE = 256;
        Entry table[TABLE_SIZE]{};

        static uint8_t hash(const Network::Address &addr) { return addr[3]; }

      public:
        void update(const Network::Address &ip, const Ethernet::Address &mac) {
            auto &entry = table[hash(ip)];
            entry.ip    = ip;
            entry.mac   = mac;
            entry.valid = true;
        }

        bool resolve(const Network::Address &ip, Ethernet::Address &mac) {
            auto &entry = table[hash(ip)];

            if (entry.valid && entry.ip == ip) {
                mac = entry.mac;
                return true;
            }

            return false;
        }

        void prepare_wait(const Network::Address &ip) { table[hash(ip)].pending++; }

        void wait(const Network::Address &ip, Spin &spin) {
            Thread::sleep(&table[hash(ip)].waiting, &spin);
        }

        void wakeup(const Network::Address &ip) {
            auto &entry = table[hash(ip)];

            while (entry.pending > 0) {
                entry.pending--;
                Thread::wakeup(&entry.waiting);
            }
        }
    };

  public:
    ARP(NIC *nic, Network *network)
        : m_nic(nic),
          m_network(network) {
        m_nic->attach(this);
    }

    void update(const unsigned char *data, size_t len) override {
        if (len < sizeof(Ethernet::Header) + sizeof(Header)) return;

        auto *eth = reinterpret_cast<const Ethernet::Header *>(data);

        if (eth->protocol() != Protocol) return;

        const auto *arp = reinterpret_cast<const Header *>(eth + 1);
        auto op         = static_cast<Opcode>(CPU::be16toh(arp->operation));

        switch (op) {
        case Opcode::REQUEST:
            handle_request(*arp);
            break;

        case Opcode::REPLY:
            handle_reply(*arp);
            break;

        default:
            break;
        }
    }

    Ethernet::Address resolve(const Network::Address &ip) {
        Ethernet::Address mac;

        while (true) {
            m_spin.acquire();

            if (m_table.resolve(ip, mac)) {
                m_spin.release();
                return mac;
            }

            m_table.prepare_wait(ip);
            send_packet(Ethernet::Broadcast, ip, Opcode::REQUEST);

            m_table.wait(ip, m_spin);
            m_spin.release();
        }
    }

  private:
    void handle_request(const Header &arp) {
        if (arp.tpa != m_network->address()) return;

        send_packet(arp.sha, arp.spa, Opcode::REPLY);
    }

    void handle_reply(const Header &arp) {
        m_spin.acquire();

        m_table.update(arp.spa, arp.sha);
        m_table.wakeup(arp.spa);

        m_spin.release();
    }

    void send_packet(const Ethernet::Address &tha, const Network::Address &tpa, Opcode op) {

        alignas(64) unsigned char buffer[sizeof(Ethernet::Header) + sizeof(Header)];

        auto *eth = new (buffer) Ethernet::Header(tha, m_nic->address(), Protocol);

        auto *arp = new (eth + 1) Header();

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
