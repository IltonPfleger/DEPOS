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
            ADDRESS0_LOW = 0x304,
            ADDRESS0_HIGH = 0x300,

        };

        struct Address {
            unsigned int high;
            unsigned int low;
        };

        static Address address() { return {Reg32(Base, ADDRESS0_HIGH) & 0xFFFF, Reg32(Base, ADDRESS0_LOW)}; }
        static void address(unsigned int high, unsigned int low) {
            Reg32(Base, ADDRESS0_HIGH) = (Reg32(Base, ADDRESS0_HIGH) & 0xFFFF0000) | (high & 0xFFFF);
            Reg32(Base, ADDRESS0_LOW) = low;
        }

        static void init() {
            TraceIn();

            if (Reg32(Base, PHY_CONTROL_STATUS) & PHY_CONTROL_STATUS_LINK_STATUS_UP) {
                Console::println("Link is Up!\n");
                if (Reg32(Base, PHY_CONTROL_STATUS) & PHY_CONTROL_STATUS_LINK_MODE_FULL_DUPLEX) {
                    Console::println("Link is Full Duplex!\n");
                } else {
                    Console::println("Link is Half Duplex!\n");
                }
                // auto [high, low] = address();
                // Console::print("Address: ");
                // Console::println("%x:", (high >> 8) & 0xFF);
                // Console::println("%x:", high & 0xFF);
                // Console::println("%x:", (low >> 24) & 0xFF);
                // Console::println("%x:", (low >> 16) & 0xFF);
                // Console::println("%x:", (low >> 8) & 0xFF);
                // Console::println("%x\n", low & 0xFF);
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
                OWN = 1 << 31,
                IOC = 1 << 30,
                VALID = 1 << 24,
                FIRST = 1 << 29,
                LAST = 1 << 28,
                TX_ERROR = 1 << 15,

                RX_AVAILABLE = OWN | IOC | VALID,
            };
        };

        enum Registers {
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
            CH0_CURRENT_APP_RX_DESCRIPTOR = 0x114c,
            CH0_CURRENT_APP_TX_DESCRIPTOR = 0x1144,
            CH0_INTERRUPT_ENABLE = 0x1134,
            CH0_INTERRUPT_STATUS = 0x1160,
        };

        enum Bits {
            CH0_INTERRUPT_ENABLE_NIE = 1 << 15,
            CH0_INTERRUPT_ENABLE_AIE = 1 << 14,
            CH0_INTERRUPT_ENABLE_RIE = 1 << 6,
            CH0_INTERRUPT_ENABLE_RBUE = 1 << 6,

            CH0_INTERRUPT_STATUS_RI = 1 << 6,
            CH0_INTERRUPT_STATUS_RBU = 1 << 7,
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
            Reg32(Base, CH0_INTERRUPT_ENABLE) |= CH0_INTERRUPT_ENABLE_NIE | CH0_INTERRUPT_ENABLE_RIE |
                                                 CH0_INTERRUPT_ENABLE_AIE | CH0_INTERRUPT_ENABLE_RBUE;
            IC::bind(Traits<GMAC0>::IRQs[0], interrupt);
            TraceOut();
        }

        static void interrupt(unsigned int) {
            unsigned int status = Reg32(Base, CH0_INTERRUPT_STATUS);

            if (status & CH0_INTERRUPT_STATUS_RI) {
                Console::out << "Receive Packet!\n";
            }

            if (status & CH0_INTERRUPT_STATUS_RBU) {
                Console::out << "Receive Buffer Unavailable!\n";
                Descriptor *next = reinterpret_cast<Descriptor *>(Reg32(Base, CH0_CURRENT_APP_RX_DESCRIPTOR));
                next->des3 = Descriptor::RX_AVAILABLE;
                Reg32(Base, CH0_RX_DESCRIPTORS_LIST_TAIL_POINTER) = reinterpret_cast<unsigned long>(next);
            }

            Reg32(Base, CH0_INTERRUPT_STATUS) = ~0U;
        }

        static void descriptors() {
            Buffer *buffers = new (Heap::SYSTEM) Buffer[k_number_of_descriptors];

            memset(m_tx_descriptors, 0, k_number_of_descriptors * sizeof(Descriptor));
            memset(m_rx_descriptors, 0, k_number_of_descriptors * sizeof(Descriptor));

            for (unsigned int i = 0; i < k_number_of_descriptors; i++) {
                auto &descriptor = m_rx_descriptors[i];
                unsigned long buffer = reinterpret_cast<unsigned long>(buffers + i);
                descriptor.des0 = static_cast<unsigned int>(buffer & 0xFFFFFFFF);
                descriptor.des1 = static_cast<unsigned int>(buffer >> 32);
                descriptor.des3 = Descriptor::OWN | Descriptor::VALID | Descriptor::IOC;
                CacheController::flush(&descriptor, sizeof(Descriptor));
            }

            Reg32(Base, CH0_RX_DESCRIPTORS_LIST_ADDR) = reinterpret_cast<unsigned long>(m_rx_descriptors) & 0xFFFFFFFF;
            Reg32(Base, CH0_RX_DESCRIPTORS_LIST_HADDR) = reinterpret_cast<unsigned long>(m_rx_descriptors) >> 32;
            Reg32(Base, CH0_RX_DESCRIPTORS_RING_LENGTH) = k_number_of_descriptors - 1;
            Reg32(Base, CH0_RX_DESCRIPTORS_LIST_TAIL_POINTER) =
                reinterpret_cast<unsigned long>(m_rx_descriptors + k_number_of_descriptors);

            Reg32(Base, CH0_TX_DESCRIPTORS_LIST_ADDR) =
                static_cast<unsigned int>(reinterpret_cast<unsigned long>(m_tx_descriptors) & 0xFFFFFFFF);
            Reg32(Base, CH0_TX_DESCRIPTORS_LIST_HADDR) =
                static_cast<unsigned int>(reinterpret_cast<unsigned long>(m_tx_descriptors) >> 32);
            Reg32(Base, CH0_TX_DESCRIPTORS_RING_LENGTH) = k_number_of_descriptors - 1;
        }

        static int receive(void *frame, unsigned int length) {

            Reg32(Base, CH0_INTERRUPT_ENABLE) &= ~CH0_INTERRUPT_ENABLE_AIE;

            unsigned long zero = reinterpret_cast<unsigned long>(m_rx_descriptors);
            unsigned long current = Reg32(Base, CH0_CURRENT_APP_RX_DESCRIPTOR);
            unsigned int i = (current - zero - 1) % k_number_of_descriptors;

            while (1) {
                Descriptor &d = m_rx_descriptors[i];
                CacheController::flush(&d, sizeof(Descriptor));
                if (!(d.des3 & Descriptor::OWN))
                    break;
                i = (i + 1) % k_number_of_descriptors;
            }

            Descriptor &d = m_rx_descriptors[i];

            unsigned long addr64 = (static_cast<unsigned long>(d.des1) << 32) | d.des0;
            unsigned short *addr = reinterpret_cast<unsigned short *>(addr64);
            unsigned long size = d.des3 & 0x3FFF;
            CacheController::flush(addr, size);

            if (size > length)
                size = length;

            memcpy(frame, addr, size);

            d.des3 = Descriptor::OWN | Descriptor::VALID | Descriptor::IOC;

            Reg32(Base, CH0_INTERRUPT_ENABLE) |= CH0_INTERRUPT_ENABLE_AIE;

            return size;
        }

        static int send(void *frame, unsigned int length) {
            unsigned long buffer = reinterpret_cast<unsigned long>(frame);
            CacheController::flush(frame, length);

            Descriptor &d = *reinterpret_cast<Descriptor *>(Reg32(Base, CH0_CURRENT_APP_TX_DESCRIPTOR));

            d.des0 = static_cast<unsigned int>(buffer & 0xFFFFFFFF);
            d.des1 = static_cast<unsigned int>(buffer >> 32);
            d.des3 = Descriptor::OWN | Descriptor::FIRST | Descriptor::LAST | (length & 0x3FFF);
            d.des2 = (length & 0x7FFF);
            CacheController::flush(&d, sizeof(Descriptor));
            Reg32(Base, CH0_TX_DESCRIPTORS_LIST_TAIL_POINTER) = reinterpret_cast<unsigned long>(&d);

            while (1) {
                CacheController::flush(&d, sizeof(Descriptor));
                if (!(d.des3 & Descriptor::OWN)) {
                    if (d.des3 & Descriptor::TX_ERROR)
                        return 0;
                    return length;
                }
            }
        }

      private:
        static constexpr unsigned int k_number_of_descriptors = 10;
        static inline Descriptor m_tx_descriptors[k_number_of_descriptors];
        static inline Descriptor m_rx_descriptors[k_number_of_descriptors];
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
    class Ethernet : public DMA {
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

        // static void send(void *frame, unsigned int length) {
        //     return DMA::send(reinterpret_cast<unsigned char *>(frame), length);
        // }

        // static void receive() { DMA::receive(); }
    };
};
