#pragma once

#include <network/GenericAddress.hpp>

namespace DEPOS {

class Ethernet {
  public:
    typedef GenericAddress<6> Address;
    typedef uint16_t Protocol;

    class Header {
      public:
        Header(const Address &d, const Address &s, const Protocol &p)
            : _destination(d),
              _source(s),
              _protocol(CPU::htobe16(p)) {}

        const Address &source() const { return _destination; }
        const Address &destination() const { return _source; }
        const Protocol &protocol() const { return CPU::be16toh(_protocol); }

      private:
        Address _destination;
        Address _source;
        Protocol _protocol;
    } __attribute__((packed));

    // Address broadcast() const { return Address({255, 255, 255, 255, 255, 255}); }

    // class Device : public AddressableNetworkDevice {
    //   public:
    //     using AddressableNetworkDevice::address;
    //     using AddressableNetworkDevice::send;

    //    virtual ~Device() {}

    //    virtual int broadcast(NetworkProtocol p, NetworkBuffer &b) override {
    //        Address d(0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF);
    //        return send(d, p, b);
    //    }

    //    virtual int send(const NetworkAddress &d, NetworkProtocol p, NetworkBuffer &b) override {
    //        new (b.start()) Header(d, address(), p);
    //        return send(b);
    //    }
    //};
};

} // namespace DEPOS
