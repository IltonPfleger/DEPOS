#pragma once

#include <network/GenericAddress.hpp>
#include <network/NetworkAddressableDevice.hpp>

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
        Protocol protocol() const { return CPU::be16toh(_protocol); }

      private:
        Address _destination;
        Address _source;
        Protocol _protocol;
    } __attribute__((packed));

    class Device : public NetworkAddressableDevice {
      public:
        using Address = Ethernet::Address;
        using Header  = Ethernet::Header;
        using NetworkDevice::send;

        virtual ~Device() {}

        NetworkBuffer *alloc(size_t length) override {
            NetworkBuffer *buffer = doAlloc(length);
            buffer->advance(sizeof(Header));
            buffer->length(length);
            return buffer;
        }

        NetworkBuffer *receive() override {
            NetworkBuffer *buffer = doReceive();
            if (buffer) {
                buffer->protocol(buffer->data<Header *>()->protocol());
                buffer->advance(sizeof(Header));
            }
            return buffer;
        }

        int send(const NetworkAddress &destination, uint16_t protocol, NetworkBuffer *buffer) override {
            buffer->rewind(sizeof(Header));
            buffer->length(buffer->length() + sizeof(Header));
            new (buffer->data()) Header(destination, address(), protocol);
            return send(buffer);
        };
    };
};

} // namespace DEPOS
