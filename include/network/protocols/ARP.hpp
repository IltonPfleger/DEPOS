#ifndef __DEPOS_NETWORK_PROTOCOLSARP_HEADER__
#define __DEPOS_NETWORK_PROTOCOLSARP_HEADER__

#include <ConditionalVariable.hpp>
#include <Thread.hpp>
#include <network/NetworkAddressResolutionService.hpp>
#include <network/NetworkDevice.hpp>
#include <network/NetworkProtocolIdentifier.hpp>
#include <utility/collections/Hash.hpp>

namespace DEPOS {

template <typename Device, typename Protocol>
class ARP : public Observer<NetworkBuffer>, public NetworkAddressResolutionService {
  public:
    enum Operation { REQUEST = 1, REPLY = 2 };

    typedef typename Device::Address HA;
    typedef Protocol::Address PA;

    struct Header {
        Header()
            : htype(CPU::htobe16(1)),
              ptype(CPU::htobe16(Protocol::Protocol)),
              hlen(sizeof(HA)),
              plen(sizeof(PA)),
              operation(CPU::htobe16(0)) {}

        uint16_t htype;
        uint16_t ptype;
        uint8_t hlen;
        uint8_t plen;
        uint16_t operation;
        HA sha;
        PA spa;
        HA dha;
        PA dpa;
    } __attribute__((packed));

    struct Entry {
        HA ha{};
        PA pa{};
        uint32_t waiting{0};
        ConditionalVariable waiters{};
        bool valid{false};
    };

    struct Hasher {
        size_t operator()(const PA &pa) const { return pa[3]; }
    };

    typedef Hash<PA, Entry, 256, Hasher> Table;

    ARP(Device *device)
        : _device(device) {
        _device->attach(this);
    }

    ~ARP() { _device->detach(this); }

    void bind(const NetworkAddress &a) { _pa = a; }

    bool resolve(const NetworkAddress &pa, uint8_t *destination) {

        while (true) {
            _lock.acquire();
            auto &entry = _table[pa];

            if (entry.valid && NetworkAddress(entry.pa) == pa) {
                memcpy(destination, &entry.ha, sizeof(HA));
                _lock.release();
                return true;
            }

            entry.waiting++;
            request(pa);
            entry.waiters.wait(&_lock);
        }
    }

  private:
    void update(NetworkBuffer buffer) {
        if (buffer.protocol() != NetworkProtocolIdentifier::ARP()) return;
        Header *header = buffer.data<Header *>();
        if (CPU::be16toh(header->operation) == REPLY) {
            onReply(*header);
        } else {
            onRequest(*header);
        }
    }

    void request(const PA &pa) {
        NetworkBuffer *buffer = _device->alloc(sizeof(Header));

        Header *header = new (buffer->data()) Header();

        header->operation = CPU::htobe16(REQUEST);

        header->sha = _device->address();
        header->spa = _pa;

        header->dha = HA();
        header->dpa = pa;

        _device->broadcast(NetworkProtocolIdentifier::ARP(), buffer);
    }

    void onReply(const Header &received) {
        _lock.acquire();
        auto &entry = _table[received.spa];
        entry.pa    = received.spa;
        entry.ha    = received.sha;
        entry.valid = true;
        entry.waiters.signalize();
        _lock.release();
    }

    void onRequest(const Header &received) {
        if (received.dpa == _pa) {
            NetworkBuffer *buffer = _device->alloc(sizeof(Header));

            Header *header = new (buffer->data()) Header();

            header->operation = CPU::htobe16(REPLY);

            header->sha = _device->address();
            header->spa = _pa;

            header->dha = received.sha;
            header->dpa = received.spa;

            _device->send(received.sha, NetworkProtocolIdentifier::ARP(), buffer);
        }
    }

  private:
    Device *_device;
    Spin _lock;
    PA _pa;
    Table _table;
};

} // namespace DEPOS

#endif
