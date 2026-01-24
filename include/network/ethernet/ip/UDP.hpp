#pragma once

#include <utils/string.hpp>

class Ethernet {

  public:
    struct Address {
        Address() = default;
        Address(const unsigned char (&data)[6]) { memcpy(m_data, data, sizeof(m_data)); }

        operator const unsigned char *() const { return m_data; }
        bool operator==(const Address &other) const { return memcmp(m_data, other.m_data, sizeof(m_data)) == 0; }
        bool operator!=(const Address &other) const { return !(*this == other); }

      private:
        unsigned char m_data[6];
    };

    struct Frame {
        typedef unsigned short EtherType;

        struct Header {
            Address destination;
            Address source;
            EtherType type;
        };

        Frame(Address destination, Address source, EtherType type, unsigned char *data, unsigned int length) {
            m_data = new unsigned char[sizeof(Header) + length];
            new (reinterpret_cast<Header *>(m_data)) Header(destination, source, type);
            memcpy(m_data + sizeof(Header), data, length);
        }

        ~Frame() { delete[] m_data; }

        operator const unsigned char *() const { return m_data; }

      private:
        unsigned char *m_data;
    };
};

class IP {
    struct Header {};
};

class UDP {};
