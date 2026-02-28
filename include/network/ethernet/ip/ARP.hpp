#pragma once

#include <Spin.hpp>
#include <network/NIC.hpp>
#include <network/ethernet/Ethernet.hpp>
#include <utils/Observer.hpp>

namespace DEPOS {

template <typename Driver> class ARP : public Observer<const unsigned char *, size_t> {
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
    static void init() { s_instance = new ARP(); }

    void update(const unsigned char *d, size_t len) override {
        if (len < sizeof(Ethernet::Header) + sizeof(Header)) return;

        const auto *eth = reinterpret_cast<const Ethernet::Header *>(d);
        if (CPU::be16toh(eth->m_type) != Ethernet::ARP) return;

        const auto *arp = reinterpret_cast<const Header *>(eth + 1);
        uint16_t op     = CPU::be16toh(arp->operation);

        if (op == REQUEST) {
            if (arp->tpa == Driver::instance()->ip()) {
                send_packet(arp->sha, arp->spa, REPLY);
            }
        } else if (op == REPLY) {
            m_spin.acquire();
            m_table.update(arp->spa, arp->sha);
            m_table.wakeup(arp->spa);
            m_spin.release();
        }
    }

    static Ethernet::MAC resolve(GenericAddress<4> pa) {
        Ethernet::MAC ha;

        while (true) {
            s_instance->m_spin.acquire();

            if (s_instance->m_table.resolve(pa, &ha)) {
                s_instance->m_spin.release();
                return ha;
            }

            s_instance->m_table.prepare_wait(pa);
            s_instance->send_packet(Ethernet::Broadcast, pa, REQUEST);

            s_instance->m_table.wait(pa, &s_instance->m_spin);
            s_instance->m_spin.release();
        }
    }

  private:
    ARP() {
        m_adapter = NIC<Driver>::instance();
        m_adapter->attach(this);
    }

    void send_packet(Ethernet::MAC tha, GenericAddress<4> tpa, uint16_t op) {
        alignas(64) unsigned char buffer[sizeof(Ethernet::Header) + sizeof(Header)];

        auto *eth = new (buffer) Ethernet::Header(tha, Driver::instance()->mac(), Ethernet::ARP);
        auto *arp = new (eth + 1) Header();

        arp->operation = CPU::htobe16(op);
        arp->sha       = Driver::instance()->mac();
        arp->spa       = Driver::instance()->ip();
        arp->tha       = (op == REQUEST) ? Ethernet::MAC("00:00:00:00:00:00") : tha;
        arp->tpa       = tpa;

        m_adapter->send(buffer, sizeof(buffer));
    }

    static inline ARP *s_instance;
    NIC<Driver> *m_adapter;
    Table m_table;
    Spin m_spin;
};

} // namespace DEPOS
