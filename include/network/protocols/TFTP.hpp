#pragma once

#include <network/protocols/UDP.hpp>

namespace DEPOS {

class TFTP {

    enum Operation : uint16_t {
        RRQ   = 1,
        WRQ   = 2,
        DATA  = 3,
        ACK   = 4,
        ERROR = 5,
        OACK  = 6,
    };

  public:
    TFTP(UDP &udp, const NetworkAddress &address)
        : _udp(udp),
          _address(address) {}

    size_t request(const char *filename, void *destination, size_t size) {
        (void)destination;
        (void)size;

        auto *buffer = _udp.alloc(256);

        size_t name_length = strlen(filename);

        auto *opcode = buffer->data<uint16_t *>();
        *opcode      = CPU::htobe16(static_cast<uint16_t>(Operation::RRQ));

        char *ptr = reinterpret_cast<char *>(opcode + 1);

        memcpy(ptr, filename, name_length + 1);
        ptr += name_length + 1;

        memcpy(ptr, "octet", 6);
        ptr += 6;

        memcpy(ptr, "blksize", 8);
        ptr += 8;

        memcpy(ptr, k_blksize_string, sizeof(k_blksize_string));
        ptr += sizeof(k_blksize_string);

        size_t total = reinterpret_cast<uint8_t *>(ptr) - reinterpret_cast<uint8_t *>(opcode);

        buffer->length(total);

        _udp.send(_address, k_port, buffer);

        _udp.free(buffer);

        //    m_semaphore.p();

        return 0;
    }

    // void update(const UDP::Datagram *datagram) {
    //     const uint8_t *tftp = datagram->data();
    //     size_t length       = datagram->length();
    //     auto opcode         = static_cast<Operation>(CPU::be16toh(*reinterpret_cast<const uint16_t *>(tftp)));
    //     m_server_port       = CPU::be16toh(datagram->header()->m_source);

    //    if (opcode == OACK) {
    //        ack(0);
    //    } else if (opcode == DATA) {
    //        uint16_t block         = CPU::be16toh(*reinterpret_cast<const uint16_t *>(tftp + 2));
    //        const uint8_t *payload = tftp + 4;
    //        size_t payload_length  = length - 4;

    //        ERROR(block != m_expected_block);
    //        ERROR(m_received_size + payload_length > m_buffer_size);

    //        memcpy(m_user_buffer + m_received_size, payload, payload_length);
    //        m_received_size += payload_length;

    //        ack(block);
    //        m_expected_block++;

    //        if (block % 64 == 0) {
    //            Console::cout << "#";
    //        }

    //        if (payload_length < k_blksize_int) {
    //            Console::cout << " [OK]" << Console::endl;
    //            m_semaphore.v();
    //        }
    //    }

    //    ERROR(opcode == Operation::ERROR);
    //}

    // void ack(uint16_t block) {
    //     auto *buffer = m_channel.alloc(4);
    //     auto *packet = reinterpret_cast<uint16_t *>(header(buffer->data()));
    //     packet[0]    = CPU::htobe16(static_cast<uint16_t>(Operation::ACK));
    //     packet[1]    = CPU::htobe16(block);
    //     m_channel.send(m_server_ip, m_server_port, buffer);
    //     m_channel.free(buffer);
    // }

  private:
    static constexpr const char *k_blksize_string     = "1468";
    static constexpr const unsigned int k_blksize_int = 1468;
    static constexpr unsigned int k_port              = 69;

  private:
    UDP &_udp;
    NetworkAddress _address;
    // Semaphore m_semaphore;
    // uint16_t m_expected_block;
    // uint8_t *m_user_buffer;
    // size_t m_buffer_size;
    // size_t m_received_size;
    // uint16_t m_server_port;
};

} // namespace DEPOS
