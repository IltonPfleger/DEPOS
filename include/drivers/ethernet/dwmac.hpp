#include <utils/Debug.hpp>
#include <utils/string.hpp>

class DMA {
    static volatile inline unsigned char *base = reinterpret_cast<volatile unsigned char *>(0x16030000);
    static volatile unsigned int &reg(int offsset) {
        return *reinterpret_cast<volatile unsigned int *>(base + offsset);
    }

    using Buffer = char[4096];
    struct Descriptor {
        uint32_t des0;
        uint32_t des1;
        uint32_t des2;
        uint32_t des3;

        enum Bits {
            OWN = 1ULL << 31,
            FD = 1ULL << 29,
            LD = 1ULL << 28,
            VALID = 1ULL << 24,
        };
    };

    enum Bits {
        SW_RESET = 0x1,
        RX_ENABLE = 0x1,
        TX_ENABLE = 0x1,
        CH0_RX_CONTROL_BUFFER_SIZE_MASK = 0x3fff,
        SYSBUS_MODE_EAME = 1ULL << 11,
        SYSBUS_MODE_BURST_LEN_16 = 1ULL << 3,
        SYSBUS_MODE_BURST_LEN_8 = 1ULL << 2,
        SYSBUS_MODE_BURST_LEN_4 = 1ULL << 1,
        SYSBUS_MODE_RD_OSR_LMT_SHIFT = 16,
    };

    enum Registers {
        MODE = 0x1000,
        SYSBUS_MODE = 0x1004,
        CH0_CONTROL = 0x1100,
        CH0_TX_CONTROL = 0x1104,
        CH0_RX_CONTROL = 0x1108,
        CH0_TX_DESCRIPTORS_LIST_HADDR = 0x1110,
        CH0_TX_DESCRIPTORS_LIST_ADDR = 0x1114,
        CH0_RX_DESCRIPTORS_LIST_HADDR = 0x1118,
        CH0_RX_DESCRIPTORS_LIST_ADDR = 0x111c,
        CH0_TX_DESCRIPTORS_LIST_TAIL_POINTER = 0x1120,
        CH0_RX_DESCRIPTORS_LIST_TAIL_POINTER = 0x1128,
        CH0_TX_DESCRIPTORS_LIST_LENGTH = 0x112c,
        CH0_RX_DESCRIPTORS_LIST_LENGTH = 0x1130,
    };

  public:
    static void init() {
        TraceIn();
        reg(MODE) |= SW_RESET;
        while (reg(MODE) & SW_RESET)
            ;
        reg(SYSBUS_MODE) |= (2 << SYSBUS_MODE_RD_OSR_LMT_SHIFT);
        reg(SYSBUS_MODE) |= SYSBUS_MODE_BURST_LEN_16 | SYSBUS_MODE_BURST_LEN_8 | SYSBUS_MODE_BURST_LEN_4;
        reg(SYSBUS_MODE) |= SYSBUS_MODE_EAME;
        descriptors();
        TraceOut();
    }

    static void descriptors() {
        Descriptor *descriptors = new (Heap::SYSTEM) Descriptor[2 * k_number_of_descriptors];
        m_tx_descriptors = descriptors;
        m_rx_descriptors = descriptors + k_number_of_descriptors;
        m_tx_buffer = reinterpret_cast<unsigned long>(new (Heap::SYSTEM) Buffer);

        memset(m_tx_descriptors, 0, k_number_of_descriptors * sizeof(Descriptor));
        memset(m_rx_descriptors, 0, k_number_of_descriptors * sizeof(Descriptor));

        for (unsigned int i = 0; i < k_number_of_descriptors; i++) {
            auto &e = m_rx_descriptors[i];
            unsigned long buffer = reinterpret_cast<unsigned long>(new (Heap::SYSTEM) Buffer);
            e.des0 = static_cast<unsigned int>(buffer & 0xFFFFFFFF); // E o upper?
            e.des3 = Descriptor::OWN | Descriptor::VALID;
        }

        reg(CH0_TX_DESCRIPTORS_LIST_HADDR) = 0;
        reg(CH0_TX_DESCRIPTORS_LIST_ADDR) = reinterpret_cast<unsigned long>(m_tx_descriptors);
        reg(CH0_TX_DESCRIPTORS_LIST_LENGTH) = k_number_of_descriptors - 1;
        reg(CH0_TX_CONTROL) |= TX_ENABLE;

        /*RX Buffer Size*/
        reg(CH0_RX_CONTROL) &= ~(CH0_RX_CONTROL_BUFFER_SIZE_MASK << 1);
        reg(CH0_RX_CONTROL) |= sizeof(Buffer) << 1;

        reg(CH0_RX_DESCRIPTORS_LIST_HADDR) = 0;
        reg(CH0_RX_DESCRIPTORS_LIST_ADDR) = reinterpret_cast<unsigned long>(m_rx_descriptors);
        reg(CH0_RX_DESCRIPTORS_LIST_LENGTH) = k_number_of_descriptors - 1;
        reg(CH0_RX_DESCRIPTORS_LIST_TAIL_POINTER) =
            reinterpret_cast<unsigned long>(m_rx_descriptors + k_number_of_descriptors);
        reg(CH0_RX_CONTROL) |= RX_ENABLE;
    }

    static void receive() {
        TraceIn();
        while (1) {
            for (unsigned int i = 0; i < k_number_of_descriptors; i++) {
                if (!(m_rx_descriptors[i].des3 & Descriptor::OWN)) {
                    break;
                }
            }
        }
        TraceOut();
    }

    // TODO: Conferir a invalidação de cache (não vi isso ainda, mas no u-boot tem uns comentários que deixam a entender
    // que isso vai ser relevante. Antes a gente viu que a SiFive-U tem coerência de cache com DMA, mas a VF2 é um
    // mistério também, pode ser que tenha ou não)
    static void send(void *packet, int length) {
        TraceIn();
        memcpy(reinterpret_cast<void *>(m_tx_buffer), packet, length);

        auto &desc = m_tx_descriptors[m_tx_desc_index];
        m_tx_desc_index = (m_tx_desc_index + 1) % k_number_of_descriptors;

        desc.des0 = static_cast<unsigned int>(m_tx_buffer & 0xFFFFFFFF);
        desc.des1 = static_cast<unsigned int>(m_tx_buffer >> 32);
        desc.des2 = length;

        // TODO: Conferir o mb() do u-boot aqui
        desc.des3 = Descriptor::OWN | Descriptor::FD | Descriptor::LD | length;

        auto *next_desc = &m_tx_descriptors[m_tx_desc_index];
        reg(CH0_TX_DESCRIPTORS_LIST_TAIL_POINTER) = reinterpret_cast<unsigned long>(next_desc);

        for (int i = 0; i < 1000000000; i++) {
            if (!(desc.des3 & Descriptor::OWN))
                return;
        }

        // Timeout

        TraceOut();
    }

  private:
    static constexpr unsigned int k_number_of_descriptors = 5;
    static inline Descriptor *m_tx_descriptors;
    static inline Descriptor *m_rx_descriptors;
    static inline unsigned long m_tx_buffer;
    static inline int m_tx_desc_index = 0;
};

class MTL {

    static volatile inline unsigned char *base = reinterpret_cast<volatile unsigned char *>(0x16030000);
    static volatile unsigned int &reg(int offsset) {
        return *reinterpret_cast<volatile unsigned int *>(base + offsset);
    }

    enum Bits {
        RX_Q0_OPERATION_MODE_RSF = 1ULL << 5,
    };

    enum Registers {
        RX_Q0_OPERATION_MODE = 0xd30,
    };

  public:
    static void init() { reg(RX_Q0_OPERATION_MODE) |= RX_Q0_OPERATION_MODE_RSF; }
};

class MAC {
    static volatile inline unsigned char *base = reinterpret_cast<volatile unsigned char *>(0x16030000);
    static volatile unsigned int &reg(int offsset) {
        return *reinterpret_cast<volatile unsigned int *>(base + offsset);
    }

    enum Bits {
        RX_QUEUE_MASK = 3,
        RX_QUEUE_ENABLE_ALL = 3,
        RX_ENABLE = 0x1,
        PACKET_FILTER_RA = 1ULL << 31,

    };

    enum Registers {
        CONFIGURATION = 0x0,
        PACKET_FILTER = 0x8,
        RX_QUEUE_CONTROL0 = 0x0a0,

    };

  public:
    static void init() {
        reg(RX_QUEUE_CONTROL0) &= ~RX_QUEUE_MASK;
        reg(RX_QUEUE_CONTROL0) |= RX_QUEUE_ENABLE_ALL;
        reg(PACKET_FILTER) |= PACKET_FILTER_RA;
    }

    static void enable() { reg(CONFIGURATION) |= RX_ENABLE; }
};

class Ethernet {
  public:
    static void init() {
        TraceIn();
        DMA::init();
        MTL::init();
        MAC::init();
        MAC::enable();
        DMA::receive();
        TraceOut();
    }
};
