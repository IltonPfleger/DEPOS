#pragma once

#include <Latch.hpp>
#include <Spin.hpp>
#include <network/NetworkAdapter.hpp>
#include <network/ethernet/Ethernet.hpp>
#include <utils/Observer.hpp>

template <typename Driver> class ARP : public Observer<const unsigned char *, size_t> {

    enum { REQUEST = 1, REPLY = 2 };

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

        Header() {
            htype = CPU::htobe16(1);
            hlen = 6;
            ptype = CPU::htobe16(0x0800);
            plen = 4;
        }
    } __attribute__((packed));

    ARP() {
        m_adapter = NetworkAdapter<Driver>::instance();
        m_adapter->attach(this);
    }

    ~ARP() { m_adapter->detach(this); }

    class Table {
        struct Entry {
            Ethernet::MAC m_ha;
            GenericAddress<4> m_pa;
            Thread::Queue m_waiting;
            bool m_valid = false;
            unsigned int m_counter = 0;
        };

      public:
        unsigned char hash(GenericAddress<4> pa) { return pa[3]; }

        void update(GenericAddress<4> pa, Ethernet::MAC ha) {
            int id = hash(pa);
            m_table[id].m_pa = pa;
            m_table[id].m_ha = ha;
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

        void wait(GenericAddress<4> pa, Spin *spin) {
            int id = hash(pa);
            m_table[id].m_counter++;
            Thread::sleep(&m_table[id].m_waiting, spin);
        }

        void wakeup(GenericAddress<4> pa) {
            int id = hash(pa);
            while (m_table[id].m_counter--) {
                Thread::wakeup(&m_table[id].m_waiting);
            }
        }

      private:
        Entry m_table[256] = {};
    };

  public:
    static void init() { s_instance = new ARP(); }

    void update(const unsigned char *d, size_t) override {
        const auto *ethernet = reinterpret_cast<const Ethernet::Header *>(d);

        if (CPU::be16toh(ethernet->m_type) != Ethernet::ARP) return;

        const auto *arp = reinterpret_cast<const Header *>(ethernet + 1);
        uint16_t operation = CPU::be16toh(arp->operation);

        if (operation == REQUEST) {
            if (arp->tpa == Driver::instance()->ip() || arp->tpa == GenericAddress<4>(255, 255, 255, 255)) {
                reply(arp->sha, arp->spa);
            }
        } else if (operation == REPLY) {
            m_spin.acquire();
            m_table.update(arp->spa, arp->sha);
            m_table.wakeup(arp->spa);
            m_spin.release();
        }
    }

    void request(GenericAddress<4> pa) {
        auto mac = Driver::instance()->mac();
        auto ip = Driver::instance()->ip();

        alignas(64) unsigned char buffer[128];
        size_t length = sizeof(Ethernet::Header) + sizeof(Header);

        auto *ethernet = new (buffer) Ethernet::Header(Ethernet::Broadcast, mac, Ethernet::ARP);
        auto *arp = new (ethernet + 1) Header();

        arp->operation = CPU::htobe16(REQUEST);
        arp->sha = mac;
        arp->spa = ip;
        arp->tha = Ethernet::MAC("00:00:00:00:00:00");
        arp->tpa = pa;

        m_adapter->send(buffer, length);
    }

    static Ethernet::MAC resolve(GenericAddress<4> pa) {
        Ethernet::MAC ha;

        while (true) {
            s_instance->m_spin.acquire();
            bool resolved = s_instance->m_table.resolve(pa, &ha);

            if (resolved) {
                s_instance->m_spin.release();
                break;
            }

            s_instance->request(pa);
            s_instance->m_table.wait(pa, &s_instance->m_spin);
        }

        return ha;
    }

  private:
    void reply(Ethernet::MAC tha, GenericAddress<4> tpa) {
        alignas(64) unsigned char buffer[128];
        auto *eth = new (buffer) Ethernet::Header(tha, Driver::instance()->mac(), Ethernet::ARP);
        auto *arp = new (eth + 1) Header();

        arp->operation = CPU::htobe16(REPLY);
        arp->sha = Driver::instance()->mac();
        arp->spa = Driver::instance()->ip();
        arp->tha = tha;
        arp->tpa = tpa;

        m_adapter->send(buffer, sizeof(Ethernet::Header) + sizeof(Header));
    }

    static inline ARP *s_instance;
    NetworkAdapter<Driver> *m_adapter;
    Table m_table;
    Spin m_spin;
};
