#pragma once

#include <Meta.hpp>
#include <architecture/Timer.hpp>
#include <hypervisor/VirtualSwitch.hpp>
#include <machine/Machine.hpp>
#include <machine/aes.h>
#include <machine/nic.h>
#include <memory/Memory.hpp>
#include <network/Ethernet.hpp>
#include <utility/observer.h>
#include <utility/ostream.h>

typedef DEPOS::Meta::GetFromTypeList<DEPOS::Traits<DEPOS::Ethernet>::Devices, 0>::Result Device;
typedef DEPOS::VirtualSwitch<Device> LocalNetwork;
typedef DEPOS::NetworkBuffer NetworkBuffer;
// typedef DEPOS::IPv4::Network<Device> LocalNetwork;
// typedef DEPOS::UDP::LocalNetwork<LocalNetwork> LocalNetwork;

class UDPNIC : public NIC<Only_Data_UDP_Wrapper>, public LocalNetwork::Observer {
    static const UInt32 KEY_SIZE = Traits<TSTP>::KEY_SIZE;
    static const UInt32 MTU      = NIC<Only_Data_UDP_Wrapper>::MTU;

    typedef AES<KEY_SIZE> _AES;
    static _AES _aes;

  public:
    UDPNIC()
        : m_network(LocalNetwork::instance()) {
        db<NIC>(TRC) << "UDPNIC::UDPNIC()" << endl;
        m_network->attach(this);
    }

    ~UDPNIC() override {
        db<NIC>(TRC) << "UDPNIC::~UDPNIC()" << endl;
        DEPOS::CPU::halt();
    }

    int send(const Address &dst, const Protocol &prot, const void *data, UInt32 size) override {
        db<NIC>(TRC) << "UDPNIC::send(dst=" << dst << ", prot=" << prot << ", data=" << data << ", size=" << size << ")"
                     << endl;
        DEPOS::CPU::halt();
        return 0;
    }

    int receive(Address *src, Protocol *prot, void *data, UInt32 size) override {
        db<NIC>(TRC) << "UDPNIC::receive(size=" << size << ")" << endl;
        DEPOS::CPU::halt();
        return 0;
    }

    Buffer *alloc(const Address &dst, const Protocol &prot, UInt32 once, UInt32 always, UInt32 payload) override {
        db<UDPNIC>(TRC) << "UDPNIC::alloc(s=" << address() << ",d=" << dst << ",p=" << hex << prot << dec
                        << ",on=" << once << ",al=" << always << ",ld=" << payload << ")" << endl;

        Buffer *buf = new Buffer(this, 0);
        db<UDPNIC>(INF) << "UDPNIC::alloc() ==> buf=" << buf << endl;
        buf->fill(once + always + payload + 14, m_configuration.address, dst, prot);
        db<UDPNIC>(INF) << "UDPNIC::alloc() ==> buf filled!" << endl;
        buf->is_microframe           = false;
        buf->trusted                 = false;
        buf->is_new                  = true;
        buf->random_backoff_exponent = 0;
        buf->microframe_count        = 0;
        buf->times_txed              = 0;
        buf->offset                  = 0;

        return buf;
    }

    int send(Buffer *buffer) override {
        db<NIC>(TRC) << "UDPNIC::send(buf=" << buffer << ") size: " << buffer->size() << endl;
        unsigned int size      = buffer->size();
        NetworkBuffer *dbuffer = m_network->alloc(size);
        dbuffer->shrink(dbuffer->offset());
        dbuffer->rewind(dbuffer->offset());
        memcpy(dbuffer->data(), buffer->frame(), size);
        m_network->send(dbuffer);
        delete buffer;
        return size;
    }

    void free(Buffer *buf) override {
        db<NIC>(TRC) << "UDPNIC::free(buf=" << buf << ")" << endl;
        delete buf;
    }

    const Address &address() override {
        db<NIC>(TRC) << "UDPNIC::address() [get]" << endl;
        return m_configuration.address;
    }

    void address(const Address &addr) override {
        db<NIC>(TRC) << "UDPNIC::address(addr=" << addr << ") [set]" << endl;
        m_configuration.address = addr;
    }

    bool reconfigure(const Configuration *c = nullptr) override {
        db<NIC>(TRC) << "UDPNIC::reconfigure(conf=" << c << ")" << endl;
        DEPOS::CPU::halt();
        return true;
    }

    const Configuration &configuration() override {
        db<NIC>(TRC) << "UDPNIC::configuration()" << endl;
        return m_configuration;
    }

    const Statistics &statistics() override {
        db<NIC>(TRC) << "UDPNIC::statistics()" << endl;
        m_statistics.time_stamp = TSC::time_stamp();
        return m_statistics;
    }

    void update(const NetworkBuffer &buffer) {
        db<NIC>(TRC) << "UDPNIC::update " << buffer.length() << endl;
        DEPOS::Ethernet::Header *header = reinterpret_cast<DEPOS::Ethernet::Header *>(buffer.start());
        UInt32 size                     = buffer.length();
        TSC::Time_Stamp ts              = TSC::time_stamp();

        Buffer *allocated = new Buffer(this, 0);
        allocated->size(size);
        memcpy(allocated->data(), buffer.start(), size);
        allocated->sfdts = ts;
        notify(header->protocol(), allocated);
    }

  private:
    static unsigned char GRP_KEY[16];

  private:
    LocalNetwork *m_network;
    Configuration m_configuration;
    Statistics m_statistics;
};
