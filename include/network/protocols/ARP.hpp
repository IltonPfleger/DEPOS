#pragma once

#include <Spin.hpp>
#include <network/NetworkDevice.hpp>
#include <utils/Observer.hpp>
#include <utils/lists/Hash.hpp>

namespace DEPOS {

template <typename Device, typename Network> class ARP : public Device::Observer {
  private:
    enum { Protocol = 0x0806 };
    enum class Opcode : uint16_t { REQUEST = 1, REPLY = 2 };

    typedef typename Device::Family Family;
    typedef typename Family::Address HA;
    typedef typename Network::Address PA;

    struct Header {
        uint16_t htype{CPU::htobe16(1)};
        uint16_t ptype{CPU::htobe16(Network::Protocol)};
        uint8_t hlen{sizeof(HA)};
        uint8_t plen{sizeof(PA)};
        uint16_t operation{0};
        HA sha;
        PA spa;
        HA tha;
        PA tpa;
    } __attribute__((packed));

    struct Entry {
        HA ha;
        PA pa;
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
        size_t operator()(const PA &addr) const { return addr[3]; }
    };

    using Table = Hash<PA, Entry, 256, Hasher>;

  public:
    ARP(Device *nic, Network *network)
        : m_nic(nic),
          m_network(network) {
        m_nic->attach(this);
    }

    HA resolve(const PA &pa) {
        HA ha;
        while (true) {
            m_spin.acquire();
            auto &entry = m_table[pa];

            if (entry.valid && entry.pa == pa) {
                ha = entry.ha;
                m_spin.release();
                return ha;
            }

            entry.pending++;
            send(HA::broadcast(), pa, Opcode::REQUEST);
            Thread::sleep(&entry.waiting, &m_spin);
        }
    }

    void update(const Device::Buffer *buffer) override {
        auto *eth = reinterpret_cast<const Family::Header *>(buffer->data());
        if (eth->protocol() != Protocol) return;

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
        entry.pa    = arp.spa;
        entry.ha    = arp.sha;
        entry.valid = true;
        entry.wakeup();
        m_spin.release();
    }

    void send(const HA &tha, const PA &tpa, Opcode op) {
        alignas(64) unsigned char data[Family::MTU];
        auto *eth      = new (data) Family::Header(tha, m_nic->address(), Protocol);
        auto *arp      = new (eth + 1) Header();
        arp->operation = CPU::htobe16(static_cast<uint16_t>(op));
        arp->sha       = m_nic->address();
        arp->spa       = m_network->address();
        arp->tha       = (op == Opcode::REQUEST) ? HA() : tha;
        arp->tpa       = tpa;
        NetworkBuffer buffer(data, sizeof(data));
        m_nic->send(&buffer);
    }

  private:
    Device *m_nic;
    Network *m_network;
    Table m_table;
    Spin m_spin;
};

} // namespace DEPOS
