#pragma once
// #include <arpa/inet.h>
// #include <netinet/in.h>
// #include <pthread.h>
// #include <sys/socket.h>
// #include <system/thread.h>
// #include <unistd.h>
//
// #include <mutex>

class UDP_Socket : public Observed {
  private:
    static constexpr const char *GRP_ADDRESS = "224.1.1.1";
    static const UInt32 PORT = 5000;

  public:
    static const UInt32 MTU = 65535 - 28; // 64KB - Headers (IPv4 and UDP)

  public:
    typedef _UTIL::Observer Observer;
    typedef _UTIL::Observed Observed;

  public:
    UDP_Socket(UInt32 port_offset = 0, bool use_receiver = true) {
        db<UDP_Socket>(TRC) << "UDP_Socket() => GRP_ADDRESS=" << GRP_ADDRESS << ":" << PORT + port_offset << endl;

        //_group_socket.sin_family      = AF_INET;
        //_group_socket.sin_addr.s_addr = inet_addr(GRP_ADDRESS);
        //_group_socket.sin_port        = CPU::_htons(PORT + port_offset);
        // db<UDP_Socket>(INF) << "Group created!" << endl;
        //_socket[0] = -1;  // send
        //_socket[1] = -1;  // receive
        //_size      = 0;
        // for (UInt32 i = 0; i < MTU; i++) {
        //    _data[i] = 0;
        //}

        // set_sockets();
        // db<UDP_Socket>(INF) << "UDP socket created!" << endl;
        // if (use_receiver) _receive_thread = new Thread(&receive_thread, this);
    }

    virtual ~UDP_Socket() {
        // if (_receive_thread) delete _receive_thread;
        // close(_socket[0]);
        // close(_socket[1]);
    }

    const unsigned char *data() { return _data; }
    TSC::Time_Stamp reception() { return _reception; }
    UInt32 size() { return _size; }

    const unsigned char *receive() {
        // int result            = 0;
        // UInt32 old_size = _size;

        // while (true) {
        //     result = read(_socket[1], _data, MTU);
        //     db<UDP_Socket>(INF) << "receive_method: Size of received DATA: " << result << endl;
        //     if (result > 0) {
        //         _reception = TSC::time_stamp();
        //         _size      = result;
        //         if (_size < old_size)
        //             for (UInt32 i = _size; i < old_size; i++) _data[i] = 0;
        //         break;
        //     }
        // }
        return _data;
    }

    int send(const unsigned char *data, UInt32 size) {
        // return sendto(_socket[0], data, size, 0, (sockaddr *)&_group_socket, sizeof(_group_socket));
    }

  private:
    int set_sockets() {
        db<UDP_Socket>(TRC) << "Creating Socket... ";
        //_socket[0] = socket(AF_INET, SOCK_DGRAM, 0);
        // int ttl    = 1;
        // if (setsockopt(_socket[0], IPPROTO_IP, IP_MULTICAST_TTL, (char *)&ttl, sizeof(ttl)) < 0) {
        //    perror("Setting TTL failed");
        //    close(_socket[0]);
        //    return 1;
        //}
        //_socket[1] = socket(AF_INET, SOCK_DGRAM, 0);
        // receive_binding();

        // return _socket[0] + _socket[1];
    }

    int receive_binding() {
        // int reuse  = 1;
        // int result = setsockopt(_socket[1], SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse));
        // if (result < 0) {
        //     db<UDP_Socket>(ERR) << "Failed to set socket opt!" << endl;
        //     return -1;
        // }

        // result = bind(_socket[1], (sockaddr *)&_group_socket, sizeof(_group_socket));
        // if (result < 0) {
        //     db<UDP_Socket>(ERR) << "Failed to bind!" << endl;
        //     return -1;
        // }
        // struct ip_mreq mreq{};
        // mreq.imr_multiaddr.s_addr = inet_addr(GRP_ADDRESS);
        // mreq.imr_interface.s_addr = INADDR_ANY;

        // result = setsockopt(_socket[1], IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq));
        // if (result < 0) {
        //     db<UDP_Socket>(ERR) << "Failed to set socket opt!" << endl;
        //     return -1;
        // }

        // return 0;
    }

    static void *receive_thread(void *p) {
        // db<UDP_Socket>(TRC) << "receive_thread()" << endl;
        // Thread::assignhandler();
        // UDP_Socket *soc = reinterpret_cast<UDP_Socket *>(p);

        // int result   = 0;
        // int old_size = 0;

        // while (true) {
        //     result = read(soc->_socket[1], soc->_data, MTU);
        //     db<UDP_Socket>(TRC) << "receive_thread: Size of received DATA: " << result << endl;
        //     if (result > 0) {
        //         soc->_reception = TSC::time_stamp();
        //         soc->_size      = result;
        //         if (result < old_size)
        //             for (int i = result; i < old_size; i++) soc->_data[i] = 0;
        //         soc->notify();
        //     }
        // }
    }

  private:
    // sockaddr_in _group_socket;
    // int _socket[2];
    UInt32 _size;
    unsigned char _data[MTU];
    TSC::Time_Stamp _reception;

    // Thread *_receive_thread;
};

#include <machine/aes.h>
#include <machine/nic.h>
#include <utility/debug.h>

class UDPNIC : public NIC<Only_Data_UDP_Wrapper>, public Observer {
  private:
    static const bool encrypt_at_nic = false;
    static const UInt32 KEY_SIZE = Traits<TSTP>::KEY_SIZE;
    static const UInt32 MTU = NIC<Only_Data_UDP_Wrapper>::MTU; // 64KB

    typedef _SYS::AES<KEY_SIZE> _AES;
    static _AES _aes;

  public:
    UDPNIC() {
        // db<UDPNIC>(TRC) << "UDPNIC()" << endl;
        // soc = new UDP_Socket();
        // soc->attach(this);
        // db<UDPNIC>(INF) << "UDP socket created!" << endl;
        //_statistics = Statistics();

        //// Set Address
        //// const UUID & id = (unsigned char[]){ 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08 }; //  Machine::uuid();
        //_configuration.address[0]      = 0;
        //_configuration.address[1]      = 0;
        //_configuration.address[2]      = 0;
        //_configuration.address[3]      = 0;
        //_configuration.address[4]      = 0;
        //_configuration.address[5]      = 0;
        //_configuration.timer_accuracy  = TSC::accuracy();   // 1;
        //_configuration.timer_frequency = TSC::frequency();  // 1000000;
        //_configuration.unit            = 1;
        // address(_configuration.address);
        //// Reset
        // reconfigure(&_configuration);
        // if (encrypt_at_nic)
        //     for (int i = 0; i < 16; ++i) GRP_KEY[i] = 0x1;
        // usleep(100000);  // let receive thread to call read() at least once
    }

    //~UDPNIC() { delete soc; }

    virtual int send(const Address &dst, const Protocol &prot, const void *data, UInt32 size) // not used
    {
        db<UDPNIC>(TRC) << "UDPNIC::send(s=" << address() << ",d=" << dst << ",p=" << hex << prot << dec << ",d=" << data
                        << ",s=" << size << ")" << endl;

        // db<UDPNIC>(TRC) << "BEFORE:" << endl;

        // for (UInt32 i = 0; i < size; ++i)
        // {
        // 	db<UDPNIC>(TRC) << reinterpret_cast<const unsigned char *>(data)[i] << ",";
        // }

        // mtx.lock();

        // unsigned char encrypted[size];
        // _aes.encrypt(reinterpret_cast<const unsigned char *>(data), reinterpret_cast<const unsigned char *>(GRP_KEY),
        // encrypted); mtx.unlock();

        // db<UDPNIC>(TRC) << "AFTER:" << endl;

        // for (UInt32 i = 0; i < size; ++i)
        // {
        // 	db<UDPNIC>(TRC) << encrypted[i] << ",";
        // }

        // int ret = soc->send(encrypted, size);

        // db<UDPNIC>(TRC) << "UNDO ENC:" << endl;
        // unsigned char decrypted[size];
        // _aes.decrypt(reinterpret_cast<const unsigned char *>(encrypted), reinterpret_cast<const unsigned char *>(GRP_KEY),
        // decrypted);

        // for (int i = 0; i < size; ++i)
        // {
        // 	db<UDPNIC>(TRC) << decrypted[i] << ",";
        // }
        db<UDPNIC>(INF) << "Done, bits sent=" << 0 << endl;
        return 0;
    }

    virtual int receive(Address *src, Protocol *prot, void *data, UInt32 size) // not used
    {
        db<UDPNIC>(TRC) << "UDPNIC::receive(s=" /*<< *src << ",p=" << hex << *prot << dec*/ << ",d=" << data << ",s=" << size
                        << ") => " << endl;

        // UInt32 ret = 0;
        // memcpy(data, buf->frame()->data<Frame>(), (buf->size() < size ? buf->size() : size));
        // free(buf);

        // return ret;
        return 0;
    }

    virtual Buffer *alloc(const Address &dst, const Protocol &prot, UInt32 once, UInt32 always, UInt32 payload) {
        // db<UDPNIC>(TRC) << "UDPNIC::alloc(s=" << address() << ",d=" << dst << ",p=" << hex << prot << dec << ",on=" << once
        // << ",al=" << always
        //                 << ",ld=" << payload << ")" << endl;

        // Buffer *buf = new Buffer(this, 0);
        // db<UDPNIC>(INF) << "UDPNIC::alloc() ==> buf=" << buf << endl;
        //// Initialize the buffer and assemble the Ethernet Frame Header
        // buf->fill(once + always + payload, _configuration.address, dst, prot);
        // db<UDPNIC>(INF) << "UDPNIC::alloc() ==> buf filled!" << endl;
        // buf->is_microframe           = false;
        // buf->trusted                 = false;
        // buf->is_new                  = true;
        // buf->random_backoff_exponent = 0;
        // buf->microframe_count        = 0;
        // buf->times_txed              = 0;
        // buf->offset                  = 0;

        // return buf;
    }

    virtual int send(Buffer *buf) {
        db<UDPNIC>(TRC) << "UDPNIC::send(buf=" << buf << ",frame=" << buf->frame() << " => " << *(buf->frame()) << endl;

        // db<UDPNIC>(TRC) << "\nBEFORE:" << endl;

        // for (int i = 0; i < buf->size(); ++i)
        // {
        // 	db<UDPNIC>(TRC) << reinterpret_cast<const unsigned char *>(buf->frame()->data<const unsigned char>())[i] << ",";
        // }
        int ret = 0;

        // if (encrypt_at_nic) {
        //     UInt32 remainders = buf->size() % KEY_SIZE;
        //     // db<UDPNIC>(TRC) << "\n (size="<< buf->size() << ",remain=" << remainders << "):" << endl;
        //     UInt32 i = 0;
        //     unsigned char encrypted[buf->size() + (remainders > 0 ? (KEY_SIZE - remainders) : 0)];
        //     mtx.lock();
        //     for (; i < buf->size() - remainders; i += KEY_SIZE)
        //         _aes.encrypt(buf->frame()->data<const unsigned char>() + i, reinterpret_cast<const unsigned char *>(GRP_KEY),
        //         encrypted + i);

        //    if (remainders) {
        //        unsigned char input[KEY_SIZE];
        //        strcpy(reinterpret_cast<char *>(input), buf->frame()->data<const char>() + i);
        //        for (UInt32 j = remainders; j < KEY_SIZE; ++j) input[j] = 0;
        //        _aes.encrypt(input, reinterpret_cast<const unsigned char *>(GRP_KEY), encrypted + i);
        //    }
        //    mtx.unlock();
        //    ret = soc->send(encrypted, buf->size() + (remainders > 0 ? (KEY_SIZE - remainders) : 0));
        //} else {
        //    ret = soc->send(buf->frame()->data<const unsigned char>(), buf->size());
        //}

        //_aes.encrypt(buf->frame()->data<const unsigned char>(), reinterpret_cast<const unsigned char *>(GRP_KEY), encrypted);

        // db<UDPNIC>(TRC) << "\nAFTER:" << endl;

        // for (i = 0; i < buf->size()+(remainders > 0 ? (KEY_SIZE-remainders):0); ++i)
        // {
        // 	db<UDPNIC>(TRC) << encrypted[i] << ",";
        // }

        // db<UDPNIC>(TRC) << "\nUNDO ENC (size="<< buf->size() << ",remain=" << remainders << "):" << endl;
        // unsigned char decrypted[buf->size()+ (remainders > 0 ? (KEY_SIZE-remainders) : 0)];
        // db<UDPNIC>(TRC) << "lets go" << endl;
        // for (i = 0; i < buf->size()+(remainders > 0 ? (KEY_SIZE-remainders):0); i+=KEY_SIZE)
        // {
        // 	db<UDPNIC>(TRC) << "step:" << i << endl;
        // 	_aes.decrypt(encrypted+i, reinterpret_cast<const unsigned char *>(GRP_KEY), decrypted+i);

        // }

        // for (i = 0; i < buf->size(); ++i)
        // {
        // 	db<UDPNIC>(TRC) << decrypted[i] << ",";
        // }

        // db<UDPNIC>(INF) << "Done, bits sent=" << ret << ", expected=" << buf->size() << endl;
        // free(buf);
        return ret;
    }

    virtual void free(Buffer *buf) {
        db<UDPNIC>(TRC) << "UDPNIC::free(buf=" << buf << ")" << endl;
        // delete buf;
    }

    virtual const Address &address() {
        db<UDPNIC>(TRC) << "UDPNIC::address()" << endl;
        // return _configuration.address;
    }

    virtual void address(const Address &addr) {
        db<UDPNIC>(TRC) << "UDPNIC::address(addr=" << addr << ")" << endl;
        //_configuration.address  = addr;
        //_configuration.selector = Configuration::ADDRESS;
        // reconfigure(&_configuration);
    }

    virtual bool reconfigure(const Configuration *c = 0) {
        db<UDPNIC>(TRC) << "UDPNIC::reconfigure(c=" << c << ")" << endl;
        return true;
    }

    virtual const Configuration &configuration() {
        db<UDPNIC>(TRC) << "UDPNIC::configuration()" << endl;
        // return _configuration;
    }

    virtual const Statistics &statistics() {
        // db<UDPNIC>(TRC) << "UDPNIC::statistics()" << endl;
        //_statistics.time_stamp = TSC::time_stamp();
        // return _statistics;
    }

    void update(typename UDP_Socket::Observed *obs) {
        // const unsigned char *data = soc->data();
        // UInt32 size         = soc->size();
        // TSC::Time_Stamp ts        = soc->reception();

        // db<UDPNIC>(TRC) << "data_received(),len=" << size << endl;
        // Protocol prot = PROTO_TSTP;
        // Buffer *buf   = new Buffer(this, 0);

        // // db<UDPNIC>(TRC) << "Content:" << endl;
        // // for (int i = 0; i < size; ++i)
        // // {
        // // 	db<UDPNIC>(TRC) << reinterpret_cast<const unsigned char *>(data)[i] << ",";
        // // }

        // if (encrypt_at_nic) {
        //     unsigned char decrypted[size];
        //     mtx.lock();
        //     for (UInt32 i = 0; i < size; i += KEY_SIZE) {
        //         _aes.decrypt(reinterpret_cast<const unsigned char *>(data) + i, reinterpret_cast<const unsigned char
        //         *>(GRP_KEY), decrypted + i);
        //     }
        //     mtx.unlock();
        //     buf->fill(size, address(), address(), prot, reinterpret_cast<const void *>(decrypted), size);
        // } else {
        //     buf->fill(size, address(), address(), prot, reinterpret_cast<const void *>(data), size);
        // }

        // // db<UDPNIC>(WRN) << "\nDecrypted:" << endl;
        // // for (int i = 0; i < size; ++i)
        // // {
        // // 	db<UDPNIC>(WRN) << reinterpret_cast<const unsigned char *>(decrypted)[i] << ",";
        // // }

        // //_aes.decrypt(reinterpret_cast<const unsigned char *>(data), reinterpret_cast<const unsigned char *>(GRP_KEY),
        // decrypted);

        // buf->sfdts = ts;

        // notify(prot, buf);  // if the UDPNIC is deleted when the receive thread called this function and the buffer was not
        // freed yet, we have a memory leak.
    }

  private:
    // Statistics _statistics;
    // Configuration _configuration;
    // std::mutex mtx;

    // static UDP_Socket *soc;
    // static unsigned char GRP_KEY[16];
};
