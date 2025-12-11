#include <machine/Machine.hpp>
#include <memory/Heap.hpp>
#include <utils/Debug.hpp>
#include <utils/string.hpp>

static constexpr unsigned long gmac0_base = 0x16030000;
static constexpr unsigned long gmac1_base = 0x16040000;
static constexpr unsigned long sys_crg_base = 0x13020000;
static constexpr unsigned long aon_crg_base = 0x17000000;
static auto &Reg(long base, long offsset) { return *reinterpret_cast<volatile unsigned int *>(base + offsset); }

class Clock {
    static constexpr unsigned int s_enable = 1 << 31;

  public:
    static void init() {
        for (int offset = 0x184; offset <= 0x1BC; offset += 4) {
            Reg(sys_crg_base, offset) |= s_enable;
        }
        for (int offset = 0x8; offset <= 0x20; offset += 4) {
            Reg(aon_crg_base, offset) |= s_enable;
        }
        Reg(sys_crg_base, 0x300) |= 0xc;
        Reg(sys_crg_base, 0x300) &= ~0xc;

        Reg(aon_crg_base, 0x38) |= 0x3;
        Reg(aon_crg_base, 0x38) &= ~0x3;
    }
};

class MDIO {
    static constexpr unsigned long s_base = gmac0_base;
    enum Register { BASE = 0x200, DATA = 0x204 };
    enum Bit {
        CR_250_300 = 0x5 << 8,
        GOC_WRITE = 0x1 << 2,
        GOC_READ = 0x3 << 2,
        GB = 0x1,
    };

  public:
    static void write(unsigned char phy, unsigned char dev, unsigned short data) {
        Reg(s_base, DATA) = data;
        Reg(s_base, BASE) = GB | GOC_WRITE | CR_250_300 | ((phy & 0x1F) << 21) | ((dev & 0x1F) << 16);
        while (Reg(s_base, BASE) & GB)
            ;
    }
    static unsigned short read(unsigned char phy, unsigned char dev) {
        Reg(s_base, BASE) = GB | GOC_READ | CR_250_300 | ((phy & 0x1F) << 21) | ((dev & 0x1F) << 16);
        while (Reg(s_base, BASE) & GB)
            ;
        return static_cast<unsigned short>(Reg(s_base, DATA) & 0xFFFF);
    }
};

class PHY {
    enum Register {
        BASIC_CONTROL = 0x0,
        BASIC_STATUS = 0x1,
        PHY_ID_1 = 0x2,
        PHY_ID_2 = 0x3,
    };
    enum Bit {
        BASIC_CONTROL_RESET = 1 << 15,
        BASIC_CONTROL_AUTO_NEGOTIATION_ENABLE = 1 << 12,
        BASIC_STATUS_AUTO_NEGOTIATION_COMPLETE = 1 << 5,
        BASIC_STATUS_LINK_STATUS = 1 << 2,
    };

  public:
    static void init() {
        for (unsigned char phy = 0; phy < 32; ++phy) {
            unsigned short id1 = MDIO::read(phy, PHY_ID_1);
            unsigned short id2 = MDIO::read(phy, PHY_ID_2);
            if (id1 == 0xFFFF && id2 == 0xFFFF) {
                continue;
            }
            MDIO::write(phy, BASIC_CONTROL, BASIC_CONTROL_RESET);
            while (MDIO::read(phy, BASIC_CONTROL) & BASIC_CONTROL_RESET)
                ;
            MDIO::write(phy, BASIC_CONTROL, BASIC_CONTROL_AUTO_NEGOTIATION_ENABLE);

            while (!(MDIO::read(phy, BASIC_STATUS) & BASIC_STATUS_AUTO_NEGOTIATION_COMPLETE))
                ;

            // Console::println("0: %x\n", MDIO::read(phy, 0x0));
            // Console::println("1: %x\n", MDIO::read(phy, 0x1));

            Console::println("%d: %x %x ", phy, id1, id2);
            if (MDIO::read(phy, BASIC_STATUS) & BASIC_STATUS_LINK_STATUS) {
                Console::println("Link is Up!\n");
                continue;
            }
            Console::println("Link is Down!\n");
        }
    }
};

class Ethernet {
  public:
    static void init() {
        TraceIn();
        Clock::init();
        PHY::init();
        TraceOut();
    }
};

// class DMA {
//     static volatile unsigned int &reg(int offsset) {
//         return *reinterpret_cast<volatile unsigned int *>(base + offsset);
//     }
//
//     using Buffer = char[4096];
//     struct Descriptor {
//         uint32_t des0;
//         uint32_t des1;
//         uint32_t des2;
//         uint32_t des3;
//
//         enum Bits {
//             OWN = 1ULL << 31,
//             VALID = 1ULL << 24,
//         };
//     };
//
//     enum Bits {
//         SW_RESET = 0x1,
//         RX_ENABLE = 0x1,
//         CH0_RX_CONTROL_BUFFER_SIZE_MASK = 0x3fff,
//         SYSBUS_MODE_EAME = 1ULL << 11,
//         SYSBUS_MODE_BURST_LEN_16 = 1ULL << 3,
//         SYSBUS_MODE_BURST_LEN_8 = 1ULL << 2,
//         SYSBUS_MODE_BURST_LEN_4 = 1ULL << 1,
//         SYSBUS_MODE_RD_OSR_LMT_SHIFT = 16,
//     };
//
//     enum Registers {
//         MODE = 0x1000,
//         SYSBUS_MODE = 0x1004,
//         CH0_CONTROL = 0x1100,
//         CH0_TX_CONTROL = 0x1104,
//         CH0_RX_CONTROL = 0x1108,
//         CH0_TX_DESCRIPTORS_LIST_HADDR = 0x1110,
//         CH0_TX_DESCRIPTORS_LIST_ADDR = 0x1114,
//         CH0_RX_DESCRIPTORS_LIST_HADDR = 0x1118,
//         CH0_RX_DESCRIPTORS_LIST_ADDR = 0x111c,
//         CH0_TX_DESCRIPTORS_LIST_TAIL_POINTER = 0x1120,
//         CH0_RX_DESCRIPTORS_LIST_TAIL_POINTER = 0x1128,
//         CH0_TX_DESCRIPTORS_LIST_LENGTH = 0x112c,
//         CH0_RX_DESCRIPTORS_LIST_LENGTH = 0x1130,
//     };
//
//   public:
//     static void init() {
//         TraceIn();
//         reg(MODE) |= SW_RESET;
//         while (reg(MODE) & SW_RESET)
//             ;
//         reg(SYSBUS_MODE) |= (2 << SYSBUS_MODE_RD_OSR_LMT_SHIFT);
//         reg(SYSBUS_MODE) |= SYSBUS_MODE_BURST_LEN_16 | SYSBUS_MODE_BURST_LEN_8 | SYSBUS_MODE_BURST_LEN_4;
//         reg(SYSBUS_MODE) |= SYSBUS_MODE_EAME;
//         descriptors();
//         TraceOut();
//     }
//
//     static void descriptors() {
//         Descriptor *descriptors = new (Heap::SYSTEM) Descriptor[2 * k_number_of_descriptors];
//         m_tx_descriptors = descriptors;
//         m_rx_descriptors = descriptors + k_number_of_descriptors;
//         memset(m_tx_descriptors, 0, k_number_of_descriptors * sizeof(Descriptor));
//         memset(m_rx_descriptors, 0, k_number_of_descriptors * sizeof(Descriptor));
//
//         for (unsigned int i = 0; i < k_number_of_descriptors; i++) {
//             auto &e = m_rx_descriptors[i];
//             unsigned long buffer = reinterpret_cast<unsigned long>(new (Heap::SYSTEM) Buffer);
//             e.des0 = static_cast<unsigned int>(buffer & 0xFFFFFFFF);
//             e.des3 = Descriptor::OWN | Descriptor::VALID;
//         }
//
//         /*RX Buffer Size*/
//         reg(CH0_RX_CONTROL) &= ~(CH0_RX_CONTROL_BUFFER_SIZE_MASK << 1);
//         reg(CH0_RX_CONTROL) |= sizeof(Buffer) << 1;
//
//         reg(CH0_RX_DESCRIPTORS_LIST_HADDR) = 0;
//         reg(CH0_RX_DESCRIPTORS_LIST_ADDR) = reinterpret_cast<unsigned long>(m_rx_descriptors);
//         reg(CH0_RX_DESCRIPTORS_LIST_LENGTH) = k_number_of_descriptors - 1;
//         reg(CH0_RX_DESCRIPTORS_LIST_TAIL_POINTER) =
//             reinterpret_cast<unsigned long>(m_rx_descriptors + k_number_of_descriptors);
//         reg(CH0_RX_CONTROL) |= RX_ENABLE;
//     }
//
//     static void receive() {
//         TraceIn();
//         while (1) {
//             for (unsigned int i = 0; i < k_number_of_descriptors; i++) {
//                 if (!(m_rx_descriptors[i].des3 & Descriptor::OWN)) {
//                     break;
//                 }
//             }
//         }
//         TraceOut();
//     }
//
//   private:
//     static constexpr unsigned int k_number_of_descriptors = 5;
//     static inline Descriptor *m_tx_descriptors;
//     static inline Descriptor *m_rx_descriptors;
// };
//
// class MTL {
//
//     static volatile inline unsigned char *base = reinterpret_cast<volatile unsigned char *>(0x16030000);
//     static volatile unsigned int &reg(int offsset) {
//         return *reinterpret_cast<volatile unsigned int *>(base + offsset);
//     }
//
//     enum Bits {
//         RX_Q0_OPERATION_MODE_RSF = 1ULL << 5,
//     };
//
//     enum Registers {
//         RX_Q0_OPERATION_MODE = 0xd30,
//     };
//
//   public:
//     static void init() { reg(RX_Q0_OPERATION_MODE) |= RX_Q0_OPERATION_MODE_RSF; }
// };
//
// class MAC {
//     static volatile inline unsigned char *base = reinterpret_cast<volatile unsigned char *>(0x16030000);
//     static volatile unsigned int &reg(int offsset) {
//         return *reinterpret_cast<volatile unsigned int *>(base + offsset);
//     }
//
//     enum Bits {
//         RX_QUEUE_MASK = 3,
//         RX_QUEUE_ENABLE_ALL = 3,
//         RX_ENABLE = 0x1,
//         PACKET_FILTER_RA = 1ULL << 31,
//
//     };
//
//     enum Registers {
//         CONFIGURATION = 0x0,
//         PACKET_FILTER = 0x8,
//         RX_QUEUE_CONTROL0 = 0x0a0,
//
//     };
//
//   public:
//     static void init() {
//         reg(RX_QUEUE_CONTROL0) &= ~RX_QUEUE_MASK;
//         reg(RX_QUEUE_CONTROL0) |= RX_QUEUE_ENABLE_ALL;
//         reg(PACKET_FILTER) |= PACKET_FILTER_RA;
//     }
//
//     static void enable() { reg(CONFIGURATION) |= RX_ENABLE; }
// };

// class Ethernet {
//   public:
//     static void init() {
//         TraceIn();
//         MAC::init();
//         MTL::init();
//         DMA::init();
//         MAC::enable();
//         DMA::receive();
//         TraceOut();
//     }
// };
