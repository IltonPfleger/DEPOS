#pragma once

#include <machine/aes.h>
#include <machine/nic.h>
#include <utility/observer.h>
#include <utility/ostream.h>

class UDP_Socket : public Observed {};

class UDPNIC : public NIC<Only_Data_UDP_Wrapper>, public Observer {
    static const UInt32 KEY_SIZE = Traits<TSTP>::KEY_SIZE;
    static const UInt32 MTU      = NIC<Only_Data_UDP_Wrapper>::MTU;

    typedef AES<KEY_SIZE> _AES;
    static _AES _aes;

  public:
    UDPNIC() {
        db<NIC>(TRC) << "UDPNIC::UDPNIC()" << endl;
        DEPOS::CPU::halt();
    }

    ~UDPNIC() override {
        db<NIC>(TRC) << "UDPNIC::~UDPNIC()" << endl;
        DEPOS::CPU::halt();
    }

    int send(const Address &dst, const Protocol &prot, const void *data, UInt32 size) override {
        db<NIC>(TRC) << "UDPNIC::send(dst=" << dst << ", prot=" << prot << ", data=" << data
                     << ", size=" << size << ")" << endl;
        DEPOS::CPU::halt();
        return 0;
    }

    int receive(Address *src, Protocol *prot, void *data, UInt32 size) override {
        db<NIC>(TRC) << "UDPNIC::receive(size=" << size << ")" << endl;
        DEPOS::CPU::halt();
        return 0;
    }

    Buffer *alloc(const Address &dst,
                  const Protocol &prot,
                  UInt32 once,
                  UInt32 always,
                  UInt32 payload) override {
        db<NIC>(TRC) << "UDPNIC::alloc(dst=" << dst << ", payload=" << payload << ")" << endl;
        Buffer *buf = new Buffer(this, 0);
        buf->fill(once + always + payload, m_configuration.address, dst, prot);
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
        db<NIC>(TRC) << "UDPNIC::send(buf=" << buf << ")" << endl;
        DEPOS::CPU::halt();
        return 0;
    }

    void free(Buffer *buf) override { db<NIC>(TRC) << "UDPNIC::free(buf=" << buf << ")" << endl; }

    const Address &address() override {
        db<NIC>(TRC) << "UDPNIC::address() [get]" << endl;
        return m_configuration.address;
    }

    void address(const Address &addr) override {
        db<NIC>(TRC) << "UDPNIC::address(addr=" << addr << ") [set]" << endl;
        DEPOS::CPU::halt();
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

    void update(typename UDP_Socket::Observed *obs) override {
        db<NIC>(TRC) << "UDPNIC::update(obs=" << obs << ")" << endl;
        DEPOS::CPU::halt();
    }

  private:
    static UDP_Socket *soc;
    static unsigned char GRP_KEY[16];

  private:
    Configuration m_configuration;
    Statistics m_statistics;
};
