#include <machine/Machine.hpp>
#include <utils/Console.hpp>

// Constantes YModem
constexpr unsigned char SOH = 0x01; // Pacote de 128 bytes
constexpr unsigned char STX = 0x02; // Pacote de 1024 bytes
constexpr unsigned char EOT = 0x04; // Fim de transmissão
constexpr unsigned char ACK = 0x06;
constexpr unsigned char NAK = 0x15;
constexpr unsigned char CAN = 0x18;
constexpr unsigned char CRC = 'C';

constexpr int PACKET_128 = 128;
constexpr int PACKET_1K = 1024;

// Funções de baixo nível (dependem da plataforma)

// Calcula CRC-16 para YModem/XModem
unsigned short crc16(const unsigned char *data, unsigned short size) {
    unsigned short crc = 0;
    for (unsigned short i = 0; i < size; i++) {
        crc ^= ((unsigned short)data[i] << 8);
        for (unsigned char j = 0; j < 8; j++) {
            if (crc & 0x8000)
                crc = (crc << 1) ^ 0x1021;
            else
                crc <<= 1;
        }
    }
    return crc;
}

// Recebe um pacote YModem
int receive_packet(unsigned char *buffer, unsigned short *size) {
    unsigned char header = IO::get();

    if (header == SOH) {
        *size = PACKET_128;
    } else if (header == STX) {
        *size = PACKET_1K;
    } else if (header == EOT) {
        return 0; // fim do arquivo
    } else if (header == CAN) {
        return -1; // cancelado
    } else {
        return -2; // pacote inválido
    }

    unsigned char packet_number = IO::get();
    unsigned char packet_number_complement = IO::get();

    // Valida número do pacote
    if ((packet_number ^ packet_number_complement) != 0xFF)
        return -2;

    // Lê dados
    for (unsigned short i = 0; i < *size; i++) {
        buffer[i] = IO::get();
    }

    // Lê CRC
    unsigned char crc_high = IO::get();
    unsigned char crc_low = IO::get();
    unsigned short crc_recv = (crc_high << 8) | crc_low;

    // Valida CRC
    if (crc16(buffer, *size) != crc_recv)
        return -2;

    return 1; // pacote ok
}

void ymodem_receive() {
    IO::put(CRC); // Solicita CRC do transmissor

    unsigned char buffer[PACKET_1K];
    unsigned int total_size = 0;

    while (true) {
        unsigned short size = 0;
        int res = receive_packet(buffer, &size);

        if (res == 0) { // fim do arquivo
            IO::put(ACK);
            break;
        } else if (res == 1) {
            total_size += size;
            IO::put(ACK); // confirma pacote
        } else if (res < 0) {
            IO::put(NAK); // erro, pede retransmissão
        }
    }

    Console::out << "SIZE: "<< total_size << "\n";
}

int main() {
    ymodem_receive();
    while (1) {
    }
    return 0;
}
