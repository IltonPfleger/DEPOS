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
    enum EtherType : uint16_t { IPv4 = 0x0800 };
    static constexpr size_t MTU = 1518;

    struct Frame {
        struct Header {
            Address m_destination;
            Address m_source;
            uint16_t m_type;
        } __attribute__((packed));

        Frame(Address destination, Address source, EtherType type) {
            new (&m_header) Header(destination, source, CPU::htobe16(type));
        }
        unsigned char *data() { return reinterpret_cast<unsigned char *>(&m_header + 1); }

        Header m_header;
    };
};

class IPv4 {
    enum Protocol : unsigned char { ICMP = 1, TCP = 6, UDP = 0x11 };
    typedef GenericAddress<4> Address;
    typedef Meta::GetFromTypeList<Traits<Ethernet>::Devices, 0>::Result NIC;
    struct PendingPacket {
        uint8_t *buffer;
        size_t received;
        size_t length;
    };
    enum { MF = 0x2000 };
    struct Header {
        uint8_t m_version;
        uint8_t m_tos;
        uint16_t m_length;
        uint16_t m_identification;
        uint16_t m_fragment;
        uint8_t m_ttl;
        uint8_t m_protocol;
        uint16_t m_checksum;
        Address m_source;
        Address m_destination;

        Header(Address destination, Address source, Protocol protocol, uint16_t length, uint8_t tos = 0,
               uint16_t identification = 0, uint16_t fragment = 0, uint8_t ttl = 64) {
            Console::print("THERE: ", fragment, '\n');
            m_version = 0x45;
            m_tos = tos;
            m_length = CPU::htobe16(sizeof(Header) + length);
            m_identification = CPU::htobe16(identification);
            m_fragment = CPU::htobe16(fragment);
            m_ttl = ttl;
            m_protocol = protocol;
            m_source = source;
            m_destination = destination;
            m_checksum = 0;
            m_checksum = checksum(this, sizeof(Header));
        }

        static uint16_t checksum(const void *data, size_t length) {
            uint32_t sum = 0;
            const uint16_t *ptr = reinterpret_cast<const uint16_t *>(data);

            for (; length > 1; length -= 2)
                sum += *ptr++;
            if (length > 0) sum += *reinterpret_cast<const uint8_t *>(ptr);

            while (sum >> 16)
                sum = (sum & 0xFFFF) + (sum >> 16);
            return static_cast<uint16_t>(~sum);
        }

    } __attribute__((packed));

  public:
    IPv4() { m_nic = NIC::instance(); }

    // void send() {
    //     Address destination({0xFF, 0xFF, 0xFF, 0xFF});
    //     Address source({0xFF, 0xFF, 0xFF, 0xFF});
    //     Protocol protocol = UDP;

    //    constexpr unsigned int k_max_frame_payload_size = 1480;

    //    constexpr unsigned int length = 4000;

    //    uint16_t sent = 0;
    //    while (sent < length) {
    //        size_t remaining = length - sent;
    //        size_t current = (remaining > k_max_frame_payload_size) ? k_max_frame_payload_size : remaining;

    //        uint16_t offset = (sent / 8);
    //        offset |= (sent + current) < length ? 0x2000 : 0;

    //        unsigned int total = sizeof(Ethernet::Frame) + sizeof(IPv4::Header) + current;
    //        uint8_t *buffer = new unsigned char[total];
    //        new (buffer + sizeof(Ethernet::Frame)) Header(destination, source, protocol, current, 0, m_id, offset);

    //        Ethernet::Address dest_mac({0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
    //        Ethernet::Address src_mac({0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF});
    //        new (buffer) Ethernet::Frame(dest_mac, src_mac, Ethernet::IPv4);

    //        NIC::instance()->send(buffer, total);

    //        sent += current;

    //        delete[] buffer;
    //    }
    //}

    size_t receive(unsigned char *destination, unsigned int length) {
        bool done = false;
        uint8_t *buffer = new unsigned char[Ethernet::MTU];

        // Variáveis de estado para remontagem
        uint16_t target_id = 0;
        bool assembling = false;
        size_t total_bytes_copied = 0;
        size_t expected_total_size = 0;
        bool seen_last_fragment = false;

        while (!done) {
            size_t received = m_nic->receive(buffer, Ethernet::MTU);
            if (received) {
                Ethernet::Frame *frame = reinterpret_cast<Ethernet::Frame *>(buffer);

                // Verifica se é IPv4
                if (frame->m_header.m_type == CPU::be16toh(Ethernet::IPv4)) {
                    Header *header = reinterpret_cast<Header *>(frame->data());

                    // Dados do cabeçalho IP
                    uint16_t fragment_field = CPU::be16toh(header->m_fragment);
                    uint16_t offset = (fragment_field & 0x1FFF) * 8;
                    bool more_fragments = (fragment_field & MF);
                    uint16_t ip_id = header->m_identification;

                    // Cálculo do tamanho do cabeçalho IP (IHL * 4) e do Payload
                    uint8_t ihl = (header->m_version & 0x0F) * 4;
                    uint16_t total_len = CPU::be16toh(header->m_length);
                    size_t payload_len = total_len - ihl;
                    uint8_t *payload_ptr = (uint8_t *)header + ihl;

                    // CASO 1: Pacote não fragmentado (Caminho feliz existente)
                    if (offset == 0 && !more_fragments && !assembling) {
                        // Verificação de segurança de buffer
                        if (payload_len <= length) {
                            memcpy(destination, payload_ptr, payload_len);
                            delete[] buffer; // IMPORTANTE: Evitar memory leak
                            return payload_len;
                        }
                        // Se buffer for pequeno demais, drop ou erro
                        break;
                    }

                    // CASO 2: Pacote Fragmentado
                    // Se ainda não estamos montando nada, começamos agora com este ID
                    if (!assembling) {
                        target_id = ip_id;
                        assembling = true;
                    }

                    // Se estamos montando, verificamos se o pacote atual pertence ao mesmo ID
                    // Se não for o mesmo ID, ignoramos (drop) nesta implementação simples de busy-wait
                    if (ip_id == target_id) {

                        // Verificação de Overflow do buffer de destino
                        if (offset + payload_len > length) {
                            // Erro: Pacote maior que o buffer fornecido
                            // Abortar ou lidar com erro
                            delete[] buffer;
                            return 0;
                        }

                        // Copia o pedaço para a posição correta
                        memcpy(destination + offset, payload_ptr, payload_len);
                        total_bytes_copied += payload_len;

                        // Se a flag MF for 0, este é o último fragmento
                        if (!more_fragments) {
                            seen_last_fragment = true;
                            expected_total_size = offset + payload_len;
                        }

                        // Verifica condição de término:
                        // Vimos o último fragmento E temos todos os bytes esperados?
                        if (seen_last_fragment && total_bytes_copied >= expected_total_size) {
                            done = true;
                            // Retorna o tamanho total remontado
                            delete[] buffer;
                            return expected_total_size;
                        }
                    } else {
                        Console::print("OI");
                    }
                }
            }
        }

        delete[] buffer;
        return 0;
    }

  private:
    uint16_t m_id;
    NIC *m_nic;
};
