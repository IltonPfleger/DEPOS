#pragma once

#include <Meta.hpp>
#include <machine/Machine.hpp>
#include <machine/Traits.hpp>

template <size_t N> struct GenericAddress {
    GenericAddress() = default;
    GenericAddress(const unsigned char (&data)[N]) { memcpy(m_data, data, sizeof(m_data)); }

    operator const unsigned char *() const { return m_data; }
    bool operator==(const GenericAddress &other) const { return memcmp(m_data, other.m_data, sizeof(m_data)) == 0; }
    bool operator!=(const GenericAddress &other) const { return !(*this == other); }

  private:
    unsigned char m_data[N];
} __attribute__((packed));

class Ethernet {

  public:
    typedef GenericAddress<6> Address;
    typedef unsigned short EtherType;

    struct Frame {
        struct Header {
            Address destination;
            Address source;
            EtherType type;
        } __attribute__((packed));

        Frame(Address destination, Address source, EtherType type) { new (this) Header(destination, source, type); }
    };
};

class IP {
    struct Header {};
};

class UDP {
    typedef Meta::GetFromTypeList<Traits<Ethernet>::Devices, 0>::Result NIC;

  public:
    static void init() {
        TraceIn();

        NIC::init();


        TraceOut();
    }
};
