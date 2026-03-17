#pragma once

#include <Meta.hpp>
#include <architecture/Timer.hpp>
#include <machine/aes.h>
#include <machine/nic.h>
#include <memory/Memory.hpp>
#include <network/ethernet/ip/UDP.hpp>
#include <utility/observer.h>
#include <utility/ostream.h>

typedef DEPOS::Meta::GetFromTypeList<DEPOS::Traits<DEPOS::Ethernet>::Devices, 0>::Result Device;
typedef DEPOS::IPv4::Network<Device> LocalNetwork;
typedef DEPOS::UDP::Channel<LocalNetwork> Channel;

class UDPNIC : public NIC<Only_Data_UDP_Wrapper>, public Channel::Observer {
    static constexpr const char *GROUP_ADDRESS = "224.1.1.1";
    static const UInt32 PORT                   = 5000;
    static const UInt32 KEY_SIZE               = Traits<TSTP>::KEY_SIZE;
    static const UInt32 MTU                    = NIC<Only_Data_UDP_Wrapper>::MTU;

    typedef AES<KEY_SIZE> _AES;
    static _AES _aes;

  public:
    UDPNIC()
        : m_channel(PORT) {
        db<NIC>(TRC) << "UDPNIC::UDPNIC()" << endl;
        m_channel.attach(this);
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
        db<NIC>(TRC) << "UDPNIC::alloc(dst=" << dst << ", payload=" << payload << ")" << endl;
        Buffer *buf = new Buffer(this, 0);
        buf->size(once + always + payload);
        buf->is_microframe           = false;
        buf->trusted                 = false;
        buf->is_new                  = true;
        buf->random_backoff_exponent = 0;
        buf->microframe_count        = 0;
        buf->times_txed              = 0;
        buf->offset                  = 0;
        return buf;
    }

    int send(Buffer *buf) override {
        db<NIC>(TRC) << "UDPNIC::send(buf=" << buf << ") size: " << buf->size() << endl;
        unsigned int size     = buf->size();
        unsigned char *buffer = new unsigned char[2048];
        memcpy(buffer + 42, buf->frame()->data<const unsigned char>(), buf->size());
        m_channel.send(GROUP_ADDRESS, PORT, buffer, buf->size());
        delete buf;
        delete[] buffer;
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
        // m_configuration.address = addr;
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

    void update(const Channel::Buffer *buffer) {
        db<NIC>(TRC) << "UDPNIC::update " << buffer->length() << endl;
        const unsigned char *data = buffer->data();
        Protocol prot             = PROTO_TSTP;
        UInt32 size               = buffer->length();
        TSC::Time_Stamp ts        = TSC::time_stamp();
        Buffer *buf               = new Buffer(this, 0);
        buf->fill(size, address(), address(), prot, reinterpret_cast<const void *>(data), size);
        buf->sfdts = ts;
        notify(prot, buf);
    }

  private:
    static unsigned char GRP_KEY[16];

  private:
    Channel m_channel;
    Configuration m_configuration;
    Statistics m_statistics;
};
