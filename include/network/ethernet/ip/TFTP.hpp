#pragma once

#include <Semaphore.hpp>
#include <network/ethernet/ip/UDP.hpp>

template <typename Driver> class TFTP : Observer<const unsigned char *, size_t> {

    enum Opcode : uint16_t {
        RRQ = 1,
        WRQ = 2,
        DATA = 3,
        ACK = 4,
        ERROR = 5,
        OACK = 6,
    };

  public:
    TFTP(IPv4::Address server_ip) : m_server_ip(server_ip) { m_connection.attach(this); }

    void *header(uint8_t *buffer) { return buffer + sizeof(Ethernet::Header) + sizeof(IPv4::Header) + sizeof(UDP::Header); }

    size_t request(const char *filename, void *buffer, size_t size) {
        m_user_buffer = reinterpret_cast<unsigned char *>(buffer);
        m_buffer_size = size;
        m_received_size = 0;
        m_expected_block = 1;
        m_server_port = 69;

        unsigned char packet[256];
        size_t name_length = strlen(filename);

        auto *opcode = reinterpret_cast<uint16_t *>(header(packet));
        *opcode = CPU::htobe16(static_cast<uint16_t>(Opcode::RRQ));

        char *ptr = reinterpret_cast<char *>(opcode + 1);

        memcpy(ptr, filename, name_length + 1);
        ptr += name_length + 1;

        memcpy(ptr, "octet", 6);
        ptr += 6;

        memcpy(ptr, "blksize", 8);
        ptr += 8;

        memcpy(ptr, k_blksize_string, sizeof(k_blksize_string));
        ptr += sizeof(k_blksize_string);

        size_t tftp_payload_size = reinterpret_cast<uint8_t *>(ptr) - reinterpret_cast<uint8_t *>(opcode);

        m_connection.send(m_server_ip, m_server_port, packet, tftp_payload_size);

        m_semaphore.p();

        return m_received_size;
    }

    void update(const unsigned char *data, size_t length) {
        const auto *udp = reinterpret_cast<const UDP::Header *>(data);
        const uint8_t *tftp = reinterpret_cast<const uint8_t *>(udp + 1);
        size_t tftp_length = length - sizeof(UDP::Header);
        auto opcode = static_cast<Opcode>(CPU::be16toh(*reinterpret_cast<const uint16_t *>(tftp)));
        m_server_port = CPU::be16toh(udp->m_source);

        if (opcode == OACK) {
            ack(0);
        } else if (opcode == DATA) {
            uint16_t block = CPU::be16toh(*reinterpret_cast<const uint16_t *>(tftp + 2));
            const uint8_t *payload = tftp + 4;
            size_t payload_length = tftp_length - 4;

            ERROR(block != m_expected_block);
            ERROR(m_received_size + payload_length > m_buffer_size);

            memcpy(m_user_buffer + m_received_size, payload, payload_length);
            m_received_size += payload_length;

            ack(block);
            m_expected_block++;

            if (block % 64 == 0) {
                Console::cout << "#";
            }

            if (payload_length < k_blksize_int) {
                Console::cout << " [OK]" << Console::endl;
                m_semaphore.v();
            }
        }

        ERROR(opcode == Opcode::ERROR);
    }

    void ack(uint16_t block) {
        unsigned char buffer[256];
        auto *packet = reinterpret_cast<uint16_t *>(header(buffer));
        packet[0] = CPU::htobe16(static_cast<uint16_t>(Opcode::ACK));
        packet[1] = CPU::htobe16(block);
        m_connection.send(m_server_ip, m_server_port, buffer, 4);
    }

  private:
    static constexpr const char *k_blksize_string = "1468";
    static constexpr const unsigned int k_blksize_int = 1468;

  private:
    UDP::Connection<Driver> m_connection;
    Semaphore m_semaphore;
    IPv4::Address m_server_ip;
    uint16_t m_expected_block;
    uint8_t *m_user_buffer;
    size_t m_buffer_size;
    size_t m_received_size;
    uint16_t m_server_port;
};
