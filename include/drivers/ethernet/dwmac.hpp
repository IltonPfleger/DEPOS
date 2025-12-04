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
            VALID = 1ULL << 24,
        };
    };

    enum Bits {
        SW_RESET = 0x1,
        RX_ENABLE = 0x1,
        CH0_RX_CONTROL_BUFFER_SIZE_MASK = 0x3fff,
    };

    enum Registers {
        MODE = 0x1000,
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
        descriptors();
        TraceOut();
    }

    static void descriptors() {
        Descriptor *descriptors = new (Heap::SYSTEM) Descriptor[2 * k_number_of_descriptors];
        m_tx_descriptors = descriptors;
        m_rx_descriptors = descriptors + k_number_of_descriptors;
        memset(m_tx_descriptors, 0, k_number_of_descriptors * sizeof(Descriptor));
        memset(m_rx_descriptors, 0, k_number_of_descriptors * sizeof(Descriptor));

        for (unsigned int i = 0; i < k_number_of_descriptors; i++) {
            auto &e = m_rx_descriptors[i];
            unsigned long buffer = reinterpret_cast<unsigned long>(new (Heap::SYSTEM) Buffer);
            e.des0 = static_cast<unsigned int>(buffer & 0xFFFFFFFF);
            e.des3 = Descriptor::OWN | Descriptor::VALID;
        }

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

  private:
    static constexpr unsigned int k_number_of_descriptors = 5;
    static inline Descriptor *m_tx_descriptors;
    static inline Descriptor *m_rx_descriptors;
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
    };

    enum Registers {
        CONFIGURATION = 0x000,
        RX_QUEUE_CONTROL0 = 0x0a0,
    };

  public:
    static void init() {
        reg(RX_QUEUE_CONTROL0) &= ~RX_QUEUE_MASK;
        reg(RX_QUEUE_CONTROL0) |= RX_QUEUE_ENABLE_ALL;
        reg(CONFIGURATION) |= RX_ENABLE;
    }
};

class Ethernet {
  public:
    static void init() {
        TraceIn();
        MAC::init();
        DMA::init();
        DMA::receive();
        TraceOut();
    }
};
