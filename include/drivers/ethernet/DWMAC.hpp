#include <drivers/Driver.hpp>
#include <machine/Machine.hpp>
#include <memory/Heap.hpp>
#include <utils/Debug.hpp>
#include <utils/string.hpp>

template <unsigned long Base> class DWMAC : Driver {
    class MDIO {
        enum Register { BASE = 0x200, DATA = 0x204 };
        enum Bit {
            CR_250_300 = 0x5 << 8,
            GOC_WRITE = 0x1 << 2,
            GOC_READ = 0x3 << 2,
            GB = 1,
        };

      public:
        static void wait() {
            while (Reg32(Base, BASE) & GB)
                ;
        }

        static void set(unsigned char phy, unsigned char dev, unsigned short data) {
            MDIO::write(phy, dev, MDIO::read(phy, dev) | data);
        }

        static void clear(unsigned char phy, unsigned char dev, unsigned short data) {
            MDIO::write(phy, dev, MDIO::read(phy, dev) & ~data);
        }

        static void write(unsigned char phy, unsigned char dev, unsigned short data) {
            Reg32(Base, DATA) = data;
            Reg32(Base, BASE) = GB | GOC_WRITE | CR_250_300 | ((phy & 0x1F) << 21) | ((dev & 0x1F) << 16);
            wait();
        }
        static unsigned short read(unsigned char phy, unsigned char dev) {
            Reg32(Base, BASE) = GB | GOC_READ | CR_250_300 | ((phy & 0x1F) << 21) | ((dev & 0x1F) << 16);
            wait();
            return static_cast<unsigned short>(Reg32(Base, DATA) & 0xFFFF);
        }

        static void write45(unsigned char phy, unsigned short reg, unsigned short data) {
            write(phy, 0x1E, reg);
            write(phy, 0x1F, data);
        }

        static void set45(unsigned char phy, unsigned short reg, unsigned short data) {
            MDIO::write45(phy, reg, MDIO::read45(phy, reg) | data);
        }

        static void clear45(unsigned char phy, unsigned short reg, unsigned short data) {
            MDIO::write45(phy, reg, MDIO::read45(phy, reg) & ~data);
        }

        static unsigned short read45(unsigned char phy, unsigned short reg) {
            write(phy, 0x1E, reg);
            return read(phy, 0x1F);
        }
    };

    class PHY {
        enum Register {
            BASIC_CONTROL = 0x0,
            BASIC_STATUS = 0x1,
            CHIP_CONFIG = 0xA001,
            RGMII_CONFIG1 = 0xA003,
            PAD_DRIVE_STRENGTH_CFG = 0xA010,
            SYNC_E_CFG = 0xA012,
            STATUS = 0x11,
            PHY_ID_1 = 0x2,
            PHY_ID_2 = 0x3,
        };
        enum Bit {
            BASIC_CONTROL_RESET = 1 << 15,
            BASIC_CONTROL_AUTO_NEGOTIATION_ENABLE = 1 << 12,
            BASIC_CONTROL_RE_AUTO_NEGOTIATION = 1 << 9,
            STATUS_SPEED_MASK = 3 << 14,
            STATUS_SPEED_1000 = 2 << 14,
            STATUS_SPEED_100 = 1 << 14,
            STATUS_SPEED_10 = 0,
            STATUS_FULL_DUPLEX = 1 << 13,
            STATUS_LINK = 1 << 10,
            BASIC_STATUS_AUTO_NEGOTIATION_COMPLETE = 1 << 5,
            RGMII_CONFIG1_TX_CLK_SEL = 1 << 14,
            CHIP_CONFIG_RXC_DELAY_ENABLE = 1 << 8,
        };

      public:
        static void init() {
            TraceIn();

            unsigned short id1 = MDIO::read(phy, PHY_ID_1);
            unsigned short id2 = MDIO::read(phy, PHY_ID_2);

            MDIO::set(phy, BASIC_CONTROL, BASIC_CONTROL_AUTO_NEGOTIATION_ENABLE | BASIC_CONTROL_RE_AUTO_NEGOTIATION);
            while (!(MDIO::read(phy, BASIC_STATUS) & BASIC_STATUS_AUTO_NEGOTIATION_COMPLETE))
                ;
            Console::println("ID: 0x%x%x\n", id1, id2);
            if (MDIO::read(phy, STATUS) & STATUS_LINK) {
                Console::println("Link is Up!\n");
                Console::println("Speed: %dMb/s\n", speed());
                if (MDIO::read(phy, STATUS) & STATUS_FULL_DUPLEX) {
                    Console::println("Link is Full Duplex!\n");
                } else {
                    Console::println("Link is Half Duplex!\n");
                }
            } else {
                Console::println("Link is Down!\n");
            }

            MDIO::set45(phy, RGMII_CONFIG1, RGMII_CONFIG1_TX_CLK_SEL);
            MDIO::clear45(phy, CHIP_CONFIG, CHIP_CONFIG_RXC_DELAY_ENABLE);
            MDIO::clear45(phy, RGMII_CONFIG1, 0xF | 0xF << 10);
            MDIO::set45(phy, RGMII_CONFIG1, 10 | 10 << 10);

            MDIO::clear45(phy, PAD_DRIVE_STRENGTH_CFG, 7 << 13);
            MDIO::set45(phy, PAD_DRIVE_STRENGTH_CFG, 6 << 13);

            MDIO::clear45(phy, PAD_DRIVE_STRENGTH_CFG, 1 << 12);
            MDIO::set45(phy, PAD_DRIVE_STRENGTH_CFG, 3 << 4);

            TraceOut();
        }

        static int speed() {
            switch ((MDIO::read(phy, STATUS) & STATUS_SPEED_MASK)) {
            case STATUS_SPEED_1000:
                return 1000;
            case STATUS_SPEED_100:
                return 100;
            case STATUS_SPEED_10:
                return 10;
            default:
                return 0;
            }
        }

      private:
        static constexpr unsigned int phy = 0;
    };

    class MAC {
      public:
        enum Bit {
            PACKET_FILTER_RECEIVE_ALL = 1 << 31,
            PACKET_FILTER_PROMISCUOUS_MODE = 1,
            CONFIGURATION_TRANSMITTER_ENABLE = 1 << 1,
            CONFIGURATION_RECEIVER_ENABLE = 1,
            RX_QUEUE_CONTROL0_QUEUE0_ENABLE = 2,
            PHY_CONTROL_STATUS_LINK_STATUS_UP = 1 << 19,
            PHY_CONTROL_STATUS_LINK_MODE_FULL_DUPLEX = 1 << 16,
            CONFIGURATION_CST = 1 << 21,

        };

        enum Register {
            CONFIGURATION = 0x0,
            PACKET_FILTER = 0x8,
            RX_QUEUE_CONTROL0 = 0xa0,
            PHY_CONTROL_STATUS = 0xf8,
        };

        static void init() {
            TraceIn();

            // TODO: Setup MAC Address

            if (Reg32(Base, PHY_CONTROL_STATUS) & PHY_CONTROL_STATUS_LINK_STATUS_UP) {
                Console::println("Link is Up!\n");
                if (Reg32(Base, PHY_CONTROL_STATUS) & PHY_CONTROL_STATUS_LINK_MODE_FULL_DUPLEX) {
                    Console::println("Link is Full Duplex!\n");
                } else {
                    Console::println("Link is Half Duplex!\n");
                }
            } else {
                Console::println("Link is Down!\n");
            }

            TraceOut();

            Reg32(Base, PACKET_FILTER) |= PACKET_FILTER_RECEIVE_ALL | PACKET_FILTER_PROMISCUOUS_MODE;
            Reg32(Base, RX_QUEUE_CONTROL0) = RX_QUEUE_CONTROL0_QUEUE0_ENABLE;
            Reg32(Base, CONFIGURATION) |= CONFIGURATION_RECEIVER_ENABLE | CONFIGURATION_TRANSMITTER_ENABLE;
            Reg32(Base, CONFIGURATION) |= CONFIGURATION_CST;
        }
    };

    class DMA {
        using Buffer = unsigned char[2048];
        struct Descriptor {
            unsigned int des0;
            unsigned int des1;
            unsigned int des2;
            unsigned int des3;

            enum Bits {
                OWN = 1ULL << 31,
                VALID = 1ULL << 24,
                FIRST = 1ULL << 29,
                LAST = 1ULL << 28,
            };
        };

        enum Register {
            MODE = 0x1000,
            SYSBUS_MODE = 0x1004,
            CH0_TX_CONTROL = 0x1104,
            CH0_RX_CONTROL = 0x1108,
            CH0_TX_DESCRIPTORS_LIST_HADDR = 0x1110,
            CH0_TX_DESCRIPTORS_LIST_ADDR = 0x1114,
            CH0_RX_DESCRIPTORS_LIST_HADDR = 0x1118,
            CH0_RX_DESCRIPTORS_LIST_ADDR = 0x111c,
            CH0_TX_DESCRIPTORS_LIST_TAIL_POINTER = 0x1120,
            CH0_RX_DESCRIPTORS_LIST_TAIL_POINTER = 0x1128,
            CH0_TX_DESCRIPTORS_RING_LENGTH = 0x112c,
            CH0_RX_DESCRIPTORS_RING_LENGTH = 0x1130,
        };

      public:
        static void reset() {
            TraceIn();
            Reg32(Base, MODE) |= 1;
            while (Reg32(Base, MODE) & 1)
                ;
            TraceOut();
        }
        static void init() {
            TraceIn();
            descriptors();
            Reg32(Base, SYSBUS_MODE) |= 1 << 11;
            Reg32(Base, CH0_TX_CONTROL) |= 1;
            Reg32(Base, CH0_RX_CONTROL) |= 1;
            TraceOut();
        }

        static void descriptors() {
            Buffer *buffers = new (Heap::SYSTEM) Buffer[k_number_of_descriptors];
            m_tx_descriptors = new (Heap::SYSTEM) Descriptor[k_number_of_descriptors];
            m_rx_descriptors = new (Heap::SYSTEM) Descriptor[k_number_of_descriptors];

            memset(m_tx_descriptors, 0, k_number_of_descriptors * sizeof(Descriptor));
            memset(m_rx_descriptors, 0, k_number_of_descriptors * sizeof(Descriptor));

            for (unsigned int i = 0; i < k_number_of_descriptors; i++) {
                auto &descriptor = m_rx_descriptors[i];
                unsigned long buffer = reinterpret_cast<unsigned long>(buffers + i);
                descriptor.des0 = static_cast<unsigned int>(buffer & 0xFFFFFFFF);
                descriptor.des1 = static_cast<unsigned int>(buffer >> 32);
                descriptor.des3 = Descriptor::OWN | Descriptor::VALID;
                CacheController::flush(&descriptor, sizeof(Descriptor));
            }

            Reg32(Base, CH0_RX_DESCRIPTORS_LIST_ADDR) =
                static_cast<unsigned int>(reinterpret_cast<unsigned long>(m_rx_descriptors) & 0xFFFFFFFF);
            Reg32(Base, CH0_RX_DESCRIPTORS_LIST_HADDR) =
                static_cast<unsigned int>(reinterpret_cast<unsigned long>(m_rx_descriptors) >> 32);
            Reg32(Base, CH0_RX_DESCRIPTORS_RING_LENGTH) = k_number_of_descriptors - 1;
            Reg32(Base, CH0_RX_DESCRIPTORS_LIST_TAIL_POINTER) =
                reinterpret_cast<unsigned long>(m_rx_descriptors + k_number_of_descriptors);

            Reg32(Base, CH0_TX_DESCRIPTORS_LIST_ADDR) =
                static_cast<unsigned int>(reinterpret_cast<unsigned long>(m_tx_descriptors) & 0xFFFFFFFF);
            Reg32(Base, CH0_TX_DESCRIPTORS_LIST_HADDR) =
                static_cast<unsigned int>(reinterpret_cast<unsigned long>(m_tx_descriptors) >> 32);
            Reg32(Base, CH0_TX_DESCRIPTORS_RING_LENGTH) = k_number_of_descriptors - 1;
        }

        static void receive() {
            TraceIn();
            while (1) {
                for (unsigned int i = 0; i < k_number_of_descriptors; i++) {
                    Descriptor &descriptor = m_rx_descriptors[i];
                    CacheController::flush(&descriptor, sizeof(Descriptor));
                    if (!(descriptor.des3 & Descriptor::OWN)) {
                        unsigned long addr64 = (static_cast<unsigned long>(descriptor.des1) << 32) | descriptor.des0;
                        unsigned short *addr = reinterpret_cast<unsigned short *>(addr64);
                        unsigned long size = descriptor.des3 & 0x3FFF;
                        Console::println("Receive: %d Bytes!\n", size);
                        CacheController::flush(addr, size);

                        for (unsigned int i = 0; i < size / 2; i++) {
                            if ((i + 1) % 16 == 0)
                                Console::print('\n');
                            Console::println("0x%x ", __builtin_bswap16(addr[i]));
                        }
                        Console::print('\n');
                        return;
                    }
                }
            }
            TraceOut();
        }

        static void send(unsigned char *frame, unsigned int length) {
            TraceIn();
            unsigned long buffer = reinterpret_cast<unsigned long>(frame);
            CacheController::flush(frame, length);
            Descriptor &descriptor = m_tx_descriptors[0];
            descriptor.des0 = static_cast<unsigned int>(buffer & 0xFFFFFFFF);
            descriptor.des1 = static_cast<unsigned int>(buffer >> 32);
            descriptor.des3 = Descriptor::OWN | Descriptor::FIRST | Descriptor::LAST | (length & 0x3FFF);
            descriptor.des2 = (length & 0x7FFF);
            CacheController::flush(&descriptor, sizeof(Descriptor));
            Reg32(Base, CH0_TX_DESCRIPTORS_LIST_TAIL_POINTER) = reinterpret_cast<unsigned long>(m_tx_descriptors + 1);

            while (1) {
                CacheController::flush(&descriptor, sizeof(Descriptor));
                if (!(descriptor.des3 & Descriptor::OWN)) {
                    Console::println("Sended!\n");
                    break;
                }
            }
            TraceOut();
        }

      private:
        static constexpr unsigned int k_number_of_descriptors = 10;
        static inline Descriptor *m_tx_descriptors;
        static inline Descriptor *m_rx_descriptors;
    };

    class MTL {
        enum Bit {
            TX_QUEUE0_OPERATION_MODE_TSF = 2,
        };

        enum Register {
            TX_QUEUE0_OPERATION_MODE = 0xd00,
        };

      public:
        static void init() {
            TraceIn();
            Reg32(Base, TX_QUEUE0_OPERATION_MODE) |= TX_QUEUE0_OPERATION_MODE_TSF;
            TraceOut();
        }
    };

  public:
    class Ethernet {
      public:
        static void init() {
            TraceIn();
            DMA::reset();
            MTL::init();
            PHY::init();
            MAC::init();
            DMA::init();
            TraceOut();
        }

        static void send(void *frame, unsigned int length) {
            return DMA::send(reinterpret_cast<unsigned char *>(frame), length);
        }

        static void receive() { DMA::receive(); }
    };
};
