#pragma once

#include <Semaphore.hpp>
#include <network/protocols/UDP.hpp>

namespace DEPOS {

class TFTP : public Observer<NetworkBuffer, uint16_t, uint16_t> {
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
          _server_address(address),
          _semaphore(0),
          _done(true) {
        _udp.attach(this);
    }

    size_t request(const char *filename, void *buffer, size_t size) {
        _buffer      = static_cast<uint8_t *>(buffer);
        _buffer_size = size;
        _received    = 0;
        _block       = 1;
        _done        = false;
        _error       = false;

        NetworkBuffer *packet = _udp.alloc(256);

        size_t name_length = strlen(filename);

        uint16_t *operation = packet->data<uint16_t *>();
        *operation          = CPU::htobe16(Operation::RRQ);

        char *ptr = reinterpret_cast<char *>(operation + 1);

        memcpy(ptr, filename, name_length + 1);
        ptr += name_length + 1;

        memcpy(ptr, "octet", 6);
        ptr += 6;

        memcpy(ptr, "blksize", 8);
        ptr += 8;

        memcpy(ptr, k_blksize_string, sizeof(k_blksize_string));
        ptr += sizeof(k_blksize_string);

        size_t total = reinterpret_cast<uint8_t *>(ptr) - reinterpret_cast<uint8_t *>(operation);

        packet->length(total);

        _udp.send(_server_address, 69, packet);

        _semaphore.p();

        if (!_error) {
            Console::cout << "\n[OK]" << Console::endl;
            return _received;
        }

        return 0;
    }

    void update(NetworkBuffer packet, uint16_t, uint16_t source) override {
        uint16_t *header   = packet.data<uint16_t *>();
        uint16_t operation = CPU::be16toh(*header);

        switch (operation) {
        case OACK: {
            ack(0, source);
            break;
        }
        case DATA: {
            uint16_t block = CPU::be16toh(*(header + 1));
            size_t length  = packet.length() - 4;
            uint8_t *data  = reinterpret_cast<uint8_t *>(header + 2);
            onData(data, block, length, source);
            break;
        }
        case ERROR: {
            onError();
            break;
        }
        }
    }

    void onData(uint8_t *data, uint16_t block, size_t length, uint16_t source) {
        if (_done) return;

        if (_received + length > _buffer_size) {
            onError();
            return;
        }

        if (block != _block) {
            ack(_block - 1, source);
            return;
        };

        _block++;
        memcpy(_buffer + _received, data, length);
        _received += length;

        ack(block, source);

        if constexpr (Trace)
            if (block % 32 == 0) Console::cout << '#';

        if (length < k_blksize_int) {
            _done = true;
            _semaphore.v();
        }
    }

    void onError() {
        _error = true;
        _semaphore.v();
    }

    void ack(uint16_t block, uint16_t source) {
        NetworkBuffer *buffer = _udp.alloc(4);
        uint16_t *payload     = buffer->data<uint16_t *>();
        payload[0]            = CPU::htobe16(Operation::ACK);
        payload[1]            = CPU::htobe16(block);
        _udp.send(_server_address, source, buffer);
    }

  private:
    static constexpr const char *k_blksize_string     = "1468";
    static constexpr const unsigned int k_blksize_int = 1468;
    static constexpr bool Trace                       = true;

  private:
    UDP &_udp;
    NetworkAddress _server_address;
    Semaphore _semaphore;

    uint8_t *_buffer;
    size_t _buffer_size;

    size_t _received;

    uint16_t _block;

    volatile bool _done;
    volatile bool _error;
};

} // namespace DEPOS
