#pragma once

#include <Spin.hpp>
#include <network/NIC.hpp>
#include <network/ethernet/Ethernet.hpp>
#include <utils/Observer.hpp>

namespace DEPOS {

template <typename NIC, typename Network>
class ARP : public Observer<const unsigned char *, size_t> {
    enum Opcode { REQUEST = 1, REPLY = 2 };

    struct Header {
        uint16_t htype;
        uint16_t ptype;
        uint8_t hlen;
        uint8_t plen;
        uint16_t operation;
        Ethernet::MAC sha;
        GenericAddress<4> spa;
        Ethernet::MAC tha;
        GenericAddress<4> tpa;

        Header()
            : htype(CPU::htobe16(1)),
              ptype(CPU::htobe16(0x0800)),
              hlen(6),
              plen(4),
              operation(0) {}
    } __attribute__((packed));

    class Table {
        struct Entry {
            Ethernet::MAC m_ha;
            GenericAddress<4> m_pa;
            Thread::Queue m_waiting;
            bool m_valid                = false;
            uint32_t m_pending_requests = 0;
        };

      public:
        uint8_t hash(GenericAddress<4> pa) { return pa[3]; }

        void update(GenericAddress<4> pa, Ethernet::MAC ha) {
            int id              = hash(pa);
            m_table[id].m_pa    = pa;
            m_table[id].m_ha    = ha;
            m_table[id].m_valid = true;
        }

        bool resolve(GenericAddress<4> pa, Ethernet::MAC *ha) {
            int id = hash(pa);
            if (m_table[id].m_valid && m_table[id].m_pa == pa) {
                *ha = m_table[id].m_ha;
                return true;
            }
            return false;
        }

        void prepare_wait(GenericAddress<4> pa) { m_table[hash(pa)].m_pending_requests++; }

        void wait(GenericAddress<4> pa, Spin *spin) {
            int id = hash(pa);
            Thread::sleep(&m_table[id].m_waiting, spin);
        }

        void wakeup(GenericAddress<4> pa) {
            int id = hash(pa);
            while (m_table[id].m_pending_requests > 0) {
                m_table[id].m_pending_requests--;
                Thread::wakeup(&m_table[id].m_waiting);
            }
        }

      private:
        Entry m_table[256] = {};
    };

  public:
    void update(const unsigned char *d, size_t len) override {
        if (len < sizeof(Ethernet::Header) + sizeof(Header)) return;

        const auto *eth = reinterpret_cast<const Ethernet::Header *>(d);
        if (CPU::be16toh(eth->m_type) != Ethernet::ARP) return;

        const auto *arp = reinterpret_cast<const Header *>(eth + 1);
        uint16_t op     = CPU::be16toh(arp->operation);

        if (op == REQUEST) {
            if (arp->tpa == m_network->address()) {
                send_packet(arp->sha, arp->spa, REPLY);
            }
        } else if (op == REPLY) {
            m_spin.acquire();
            m_table.update(arp->spa, arp->sha);
            m_table.wakeup(arp->spa);
            m_spin.release();
        }
    }

    Ethernet::MAC resolve(GenericAddress<4> pa) {
        Ethernet::MAC ha;

        while (true) {
            m_spin.acquire();

            if (m_table.resolve(pa, &ha)) {
                m_spin.release();
                return ha;
            }

            m_table.prepare_wait(pa);
            send_packet(Ethernet::Broadcast, pa, REQUEST);

            m_table.wait(pa, &m_spin);
            m_spin.release();
        }
    }

    ARP(NIC *nic, Network *network)
        : m_nic(nic),
          m_network(network) {
        m_nic->attach(this);
    }

  private:
    void send_packet(Ethernet::MAC tha, GenericAddress<4> tpa, uint16_t op) {
        alignas(64) unsigned char buffer[sizeof(Ethernet::Header) + sizeof(Header)];

        auto *eth = new (buffer) Ethernet::Header(tha, m_nic->address(), Ethernet::ARP);
        auto *arp = new (eth + 1) Header();

        arp->operation = CPU::htobe16(op);
        arp->sha       = m_nic->address();
        arp->spa       = m_network->address();
        arp->tha       = (op == REQUEST) ? Ethernet::MAC("00:00:00:00:00:00") : tha;
        arp->tpa       = tpa;

        m_nic->send(buffer, sizeof(buffer));
    }

    NIC *m_nic;
    Network *m_network;
    Table m_table;
    Spin m_spin;
};

} // namespace DEPOS
