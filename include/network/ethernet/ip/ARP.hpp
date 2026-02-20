#pragma once

#include <Semaphore.hpp>
#include <Spin.hpp>
#include <network/ethernet/Ethernet.hpp>
#include <network/ethernet/NIC.hpp>
#include <utils/Lists.hpp>
#include <utils/Observer.hpp>
#include <utils/Singleton.hpp>

template <typename Driver> class ARP : public Observer<const unsigned char *, size_t>, public Singleton<ARP<Driver>> {
    friend Singleton<ARP<Driver>>;

    struct Header {
        uint16_t htype = 0x0100; // Hardware Type: Ethernet (Big Endian)
        uint16_t ptype = 0x0008; // Protocol Type: IPv4 (Big Endian)
        uint8_t hlen = 6;        // Hardware length (MAC)
        uint8_t plen = 4;        // Protocol length (IP)
        uint16_t operation;      // 0x0100 para Request, 0x0200 para Reply (Big Endian)
        Ethernet::Address sha;   // Sender Hardware Address
        GenericAddress<4> spa;   // Sender Protocol Address
        Ethernet::Address tha;   // Target Hardware Address (0 no Request)
        GenericAddress<4> tpa;   // Target Protocol Address
    } __attribute__((packed));

    struct Pending {
        Header *header;
        Thread::Queue m_queue;

        Pending(Header *h) : header(h) {}
    };

    typedef Node<Pending *> Link;

    ARP(NIC<Driver> *nic = NIC<Driver>::instance()) : m_nic(nic) {
        m_nic->attach(this);
        m_pending = Ethernet::Address("0.0.0.0.0.0");
    }

    ~ARP() {
        m_nic->detach(this);
        NIC<Driver>::release();
    }

  public:
    Ethernet::Address resolve(GenericAddress<4> ip) {
        Ethernet::Address my_mac = Driver::instance()->mac();
        Ethernet::Address broadcast("255:255:255:255:255:255");
        Ethernet::Address zeros("0.0.0.0.0.0");
        GenericAddress<4> my_ip = Driver::instance()->ip();

        unsigned char packet[sizeof(Ethernet::Header) + sizeof(Header) + 64];

        Ethernet::Header *ethernet = new (packet) Ethernet::Header(broadcast, my_mac, Ethernet::EtherType::ARP);
        Header *header = new (ethernet + 1) Header();

        header->operation = 0x0100;
        header->sha = my_mac;
        header->spa = my_ip;
        header->tha = zeros;
        header->tpa = ip;

        while (m_pending == zeros) {
            Driver::instance()->send(packet, sizeof(packet));
            volatile unsigned int i = 100'000'000;
            while (i) {
                i = i - 1;
            }
        }

        return m_pending;
    }

    void update(const unsigned char *data, size_t) {
        const Ethernet::Header *ethernet = reinterpret_cast<const Ethernet::Header *>(data);

        if (ethernet->m_type != Ethernet::EtherType::ARP) return;

        const Header *header = reinterpret_cast<const Header *>(ethernet + 1);

        if (header->operation == 0x0100) {
            GenericAddress<4> my_ip = Driver::instance()->ip();
            Ethernet::Address my_mac = Driver::instance()->mac();

            if (header->tpa == my_ip) {
                unsigned char packet[sizeof(Ethernet::Header) + sizeof(Header) + 64];

                Ethernet::Header *reply_eth = new (packet) Ethernet::Header(header->sha, my_mac, Ethernet::EtherType::ARP);
                Header *reply_arp = new (reply_eth + 1) Header();

                reply_arp->operation = 0x0200;
                reply_arp->sha = my_mac;
                reply_arp->spa = my_ip;
                reply_arp->tha = header->sha;
                reply_arp->tpa = header->spa;

                Driver::instance()->send(packet, sizeof(packet));
            }
        } else if (header->operation == 0x0200) {
            m_pending = header->sha;
        }
    }

  private:
    // volatile bool m_running = true;
    NIC<Driver> *m_nic;
    Ethernet::Address m_pending;
    // LIFO<Link> m_pending;
    // Spin m_spin;
};
