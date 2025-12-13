#include <machine/Machine.hpp>
#include <memory/Heap.hpp>
#include <utils/Debug.hpp>
#include <utils/string.hpp>

static constexpr unsigned long gmac0_base = 0x16030000;
static constexpr unsigned long gmac1_base = 0x16040000;
static constexpr unsigned long gmac_base = gmac1_base;
static constexpr unsigned long sys_crg_base = 0x13020000;
static constexpr unsigned long aon_crg_base = 0x17000000;
static auto &Reg(long base, long offsset) { return *reinterpret_cast<volatile unsigned int *>(base + offsset); }

#define L2_CACHE_FLUSH64 0x200
#define L2_CACHE_BASE_ADDR 0x2010000
#define CONFIG_SYS_CACHELINE_SIZE 64
#define mb() asm __volatile__("fence iorw, iorw" : : : "memory")
void flush(void *ptr, unsigned long size) {
    unsigned long line;
    unsigned long start = reinterpret_cast<unsigned long>(ptr);
    unsigned long end = start + size;
    volatile unsigned long *flush64;

    flush64 = (volatile unsigned long *)(L2_CACHE_BASE_ADDR + L2_CACHE_FLUSH64);

    mb();
    for (line = start; line < end; line += CONFIG_SYS_CACHELINE_SIZE) {
        (*flush64) = line;
        mb();
    }

    return;
}

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
    enum Register { BASE = 0x200, DATA = 0x204 };
    enum Bit {
        CR_250_300 = 0x5 << 8,
        GOC_WRITE = 0x1 << 2,
        GOC_READ = 0x3 << 2,
        GB = 1,
    };

  public:
    static void wait() {
        while (Reg(gmac_base, BASE) & GB)
            ;
    }

    static void set(unsigned char phy, unsigned char dev, unsigned short data) {
        MDIO::write(phy, dev, MDIO::read(phy, dev) | data);
    }

    static void write(unsigned char phy, unsigned char dev, unsigned short data) {
        Reg(gmac_base, DATA) = data;
        Reg(gmac_base, BASE) = GB | GOC_WRITE | CR_250_300 | ((phy & 0x1F) << 21) | ((dev & 0x1F) << 16);
        wait();
    }
    static unsigned short read(unsigned char phy, unsigned char dev) {
        Reg(gmac_base, BASE) = GB | GOC_READ | CR_250_300 | ((phy & 0x1F) << 21) | ((dev & 0x1F) << 16);
        wait();
        return static_cast<unsigned short>(Reg(gmac_base, DATA) & 0xFFFF);
    }

    static void write45(unsigned char phy, unsigned short reg, unsigned short data) {
        write(phy, 0x1E, reg);
        write(phy, 0x1F, data);
    }

    static void set45(unsigned char phy, unsigned short reg, unsigned short data) {
        MDIO::write45(phy, reg, MDIO::read45(phy, reg) | data);
    }

    static unsigned short read45(unsigned char phy, unsigned short reg) {
        write(phy, 0x1E, reg);
        return read(phy, 0x1F);
    }
};

class MMC {
    enum Register {
        RX_PACKETS_COUNT_GOOD_BAD = 0x780,
        TX_PACKETS_COUNT_GOOD_BAD = 0x718,
        TX_PACKETS_COUNT_GOOD = 0x768,
        TX_BROADCAST_PACKETS_COUNT_GOOD = 0x71c,
    };

  public:
    static unsigned int received() { return Reg(gmac_base, RX_PACKETS_COUNT_GOOD_BAD); }
    static unsigned int not_transmitted() {
        return Reg(gmac_base, TX_PACKETS_COUNT_GOOD_BAD) - Reg(gmac_base, TX_PACKETS_COUNT_GOOD);
    }
    static unsigned int transmitted() { return Reg(gmac_base, TX_PACKETS_COUNT_GOOD); }
    static unsigned int broadcast() { return Reg(gmac_base, TX_BROADCAST_PACKETS_COUNT_GOOD); }
};

class PHY {
    enum Register {
        BASIC_CONTROL = 0x0,
        BASIC_STATUS = 0x1,
        RGMII_CONFIG1 = 0xA003,
        STATUS = 0x11,
        PHY_ID_1 = 0x2,
        PHY_ID_2 = 0x3,
    };
    enum Bit {
        BASIC_CONTROL_RESET = 1 << 15,
        BASIC_CONTROL_LOOPBACK = 1 << 14,
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
    };

  public:
    static void init() {
        TraceIn();

        unsigned short id1 = MDIO::read(phy, PHY_ID_1);
        unsigned short id2 = MDIO::read(phy, PHY_ID_2);

        MDIO::set(phy, BASIC_CONTROL, BASIC_CONTROL_RESET);
        while (MDIO::read(phy, BASIC_CONTROL) & BASIC_CONTROL_RESET)
            ;
        MDIO::set(phy, BASIC_CONTROL, BASIC_CONTROL_AUTO_NEGOTIATION_ENABLE | BASIC_CONTROL_RE_AUTO_NEGOTIATION);
        while (!(MDIO::read(phy, BASIC_STATUS) & BASIC_STATUS_AUTO_NEGOTIATION_COMPLETE))
            ;
        Console::println("Phy ID: %x %x\n", id1, id2);
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

        // MDIO::set45(phy, RGMII_CONFIG1, RGMII_CONFIG1_TX_CLK_SEL);
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

    static constexpr unsigned int phy = 0;
};

class DMA {
    using Buffer = unsigned char[2048];
    struct Descriptor {
        uint32_t des0;
        uint32_t des1;
        uint32_t des2;
        uint32_t des3;

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
        CH0_CONTROL = 0x1100,
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

    enum Bit {
        MODE_SOFTWARE_RESET = 1,
        SYSBUS_MODE_EAME = 1ull << 11,
        CH0_RX_CONTROL_ENABLE = 1,
        CH0_TX_CONTROL_ENABLE = 1,
    };

  public:
    static void reset() {
        TraceIn();
        Reg(gmac_base, MODE) |= MODE_SOFTWARE_RESET;
        while (Reg(gmac_base, MODE) & MODE_SOFTWARE_RESET)
            ;
        TraceOut();
    }
    static void init() {
        TraceIn();
        Reg(gmac_base, SYSBUS_MODE) |= SYSBUS_MODE_EAME;
        descriptors();
        Reg(gmac_base, CH0_RX_CONTROL) |= CH0_RX_CONTROL_ENABLE;
        Reg(gmac_base, CH0_TX_CONTROL) |= CH0_TX_CONTROL_ENABLE;
        TraceOut();
    }

    static void descriptors() {
        Descriptor *descriptors = new (Heap::SYSTEM) Descriptor[2 * k_number_of_descriptors];
        memset(descriptors, 0, 2 * k_number_of_descriptors * sizeof(Descriptor));

        m_tx_descriptors = descriptors;
        m_rx_descriptors = descriptors + k_number_of_descriptors;

        for (unsigned int i = 0; i < k_number_of_descriptors; i++) {
            auto &e = m_rx_descriptors[i];
            unsigned long buffer = reinterpret_cast<unsigned long>(new (Heap::SYSTEM) Buffer);
            e.des0 = static_cast<unsigned int>(buffer & 0xFFFFFFFF);
            e.des1 = static_cast<unsigned int>(buffer >> 32);
            e.des3 = Descriptor::OWN | Descriptor::VALID;
        }

        Reg(gmac_base, CH0_RX_DESCRIPTORS_LIST_ADDR) =
            static_cast<unsigned int>(reinterpret_cast<unsigned long>(m_rx_descriptors) & 0xFFFFFFFF);
        Reg(gmac_base, CH0_RX_DESCRIPTORS_LIST_HADDR) =
            static_cast<unsigned int>(reinterpret_cast<unsigned long>(m_rx_descriptors) >> 32);
        Reg(gmac_base, CH0_RX_DESCRIPTORS_RING_LENGTH) = k_number_of_descriptors - 1;
        Reg(gmac_base, CH0_RX_DESCRIPTORS_LIST_TAIL_POINTER) =
            reinterpret_cast<unsigned long>(m_rx_descriptors + k_number_of_descriptors);

        Reg(gmac_base, CH0_TX_DESCRIPTORS_LIST_ADDR) =
            static_cast<unsigned int>(reinterpret_cast<unsigned long>(m_tx_descriptors) & 0xFFFFFFFF);
        Reg(gmac_base, CH0_TX_DESCRIPTORS_LIST_HADDR) =
            static_cast<unsigned int>(reinterpret_cast<unsigned long>(m_tx_descriptors) >> 32);
        Reg(gmac_base, CH0_TX_DESCRIPTORS_RING_LENGTH) = k_number_of_descriptors - 1;

        // Reg(gmac_base, CH0_TX_DESCRIPTORS_LIST_TAIL_POINTER) = reinterpret_cast<unsigned long>(m_tx_descriptors);
        //  Reg(gmac_base, CH0_TX_DESCRIPTORS_LIST_TAIL_POINTER) =
        //      reinterpret_cast<unsigned long>(m_tx_descriptors + k_number_of_descriptors + 1);
    }

    static void debug() {
        unsigned char *frame = new (Heap::SYSTEM) Buffer;
        size_t off = 0;

        unsigned char dst[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
        for (int i = 0; i < 6; i++)
            frame[off++] = dst[i];

        unsigned char src[6] = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55};
        for (int i = 0; i < 6; i++)
            frame[off++] = src[i];

        frame[off++] = 0x08;
        frame[off++] = 0x00;

        const char payload[] = "HELLO FRAME";
        for (unsigned int i = 0; i < sizeof(payload) - 1; i++)
            frame[off++] = payload[i];

        for (; off < 1500; off++)
            frame[off] = 0;

        int length = off;
        unsigned long buffer = reinterpret_cast<unsigned long>(frame);
        Console::println("%x %d\n", frame, length);
        flush(frame, length);
        Descriptor &descriptor = m_tx_descriptors[0];
        descriptor.des0 = static_cast<unsigned int>(buffer & 0xFFFFFFFF);
        descriptor.des1 = static_cast<unsigned int>(buffer >> 32);
        descriptor.des3 = Descriptor::OWN | Descriptor::FIRST | Descriptor::LAST | (length & 0x3FFF);
        descriptor.des2 = (length & 0x7FFF);
        flush(&descriptor, sizeof(Descriptor));
        Reg(gmac_base, CH0_TX_DESCRIPTORS_LIST_TAIL_POINTER) = reinterpret_cast<unsigned long>(m_tx_descriptors + 1);
        enum {
            DEBUG_STATUS0 = 0x100c,
            CH0_STATUS = 0x1160,
        };
        while (1) {
            Console::println("CH0_STATUS: %x\n", Reg(gmac_base, CH0_STATUS));
            Console::println("Bad: %d | Good: %d | Broadcast: %d\n", MMC::not_transmitted(), MMC::transmitted(),
                             MMC::broadcast());
            // Console::println("DMA PHY: %x\n", Reg(gmac_base, 0xf8));
            // Console::println("TX UNDERFLOW: %x\n", Reg(gmac_base, 0x748));
            // Console::println("TX LATE COLLISION: %x\n", Reg(gmac_base, 0x758));
            // Console::println("TX EXCESSIVE COLLISION: %x\n", Reg(gmac_base, 0x75c));
            // Console::println("TX CARRIER: %x\n", Reg(gmac_base, 0x760));
            // Console::println("TX DEFERRAL ERROR: %x\n", Reg(gmac_base, 0x76c));
            // Console::println("MTL TX Q0 DEBUG: %x\n", Reg(gmac_base, 0xd08));
            // Console::println("MAC RX TX STATUS: %x\n", Reg(gmac_base, 0xb8));
            // Console::println("MAC PHY CONTROL STATUS: %x\n", Reg(gmac_base, 0xf8));
            // Console::println("MAC DEBUG: %x\n", Reg(gmac_base, 0x114));
            // Console::println("MMC TX IRQ: %x\n", Reg(gmac_base, 0x708));
            // Console::println("MMC TX OCT GOOD BAD: %x\n", Reg(gmac_base, 0x714));
        }
    }
    // while (1) {
    //     for (unsigned int i = 0; i < k_number_of_descriptors; i++) {
    //         Descriptor descriptor = m_rx_descriptors[i];
    //         if (!(descriptor.des3 & Descriptor::OWN)) {
    //             Console::println("Used Descriptor!\n");
    //             unsigned long addr64 = (static_cast<unsigned long>(descriptor.des1) << 32) |
    //                                    static_cast<unsigned long>(descriptor.des0);

    //            unsigned short *addr = reinterpret_cast<unsigned short *>(addr64);

    //            for (unsigned int i = 0; i < sizeof(Buffer) / 2; i++) {
    //                if (i % 100 == 0)
    //                    Console::print("\n");
    //                Console::println("%x ", addr[i]);
    //            }
    //            Console::print("\n");
    //            return;
    //        }
    //        //    for (unsigned int i = 0; i < sizeof(Buffer); i++) {
    //        //        if (i % 100 == 0)
    //        //            Console::print("\n");
    //        //        Console::println("%x ", *addr);
    //        //        addr++;
    //        //    }
    //        //}
    //        // Console::print("\n");
    //        // Console::println("DEBUG_STATUS0: %x\n", Reg(gmac_base, DEBUG_STATUS0));
    //    }
    //}
    //}

    // static void receive() {
    //     TraceIn();
    //     while (1) {
    //         for (unsigned int i = 0; i < k_number_of_descriptors; i++) {
    //             if (!(m_rx_descriptors[i].des3 & Descriptor::OWN)) {
    //                 break;
    //             }
    //         }
    //     }
    //     TraceOut();
    // }

  private:
    static constexpr unsigned int k_number_of_descriptors = 10;
    static inline Descriptor *m_tx_descriptors;
    static inline Descriptor *m_rx_descriptors;
};

class MAC {
    enum Bit {
        PACKET_FILTER_RECEIVE_ALL = 1 << 31,
        PACKET_FILTER_PROMISCUOUS_MODE = 1,
        CONFIGURATION_TRANSMITTER_ENABLE = 2,
        CONFIGURATION_RECEIVER_ENABLE = 1,
        CONFIGURATION_FULL_DUPLEX = 1 << 13,
        RX_QUEUE_CONTROL0_QUEUE0_ENABLE = 2,
        PHY_CONTROL_STATUS_LINK_STATUS_UP = 1 << 19,
        PHY_CONTROL_STATUS_LINK_MODE_FULL_DUPLEX = 1 << 16,
    };

    enum Register {
        CONFIGURATION = 0x0,
        PACKET_FILTER = 0x8,
        RX_QUEUE_CONTROL0 = 0xa0,
        PHY_CONTROL_STATUS = 0xf8,
        ADDRESS0_HIGH = 0x300,
        ADDRESS0_LOW = 0x304,
    };

  public:
    static void init() {
        TraceIn();
        if (Reg(gmac_base, PHY_CONTROL_STATUS) & PHY_CONTROL_STATUS_LINK_STATUS_UP) {
            Console::println("Link is Up!\n");
            if (Reg(gmac_base, PHY_CONTROL_STATUS) & PHY_CONTROL_STATUS_LINK_MODE_FULL_DUPLEX) {
                Console::println("Link is Full Duplex!\n");
                address(0x001A2B3C4D5E);
                Console::println("Address: %x\n", address());
            } else {
                Console::println("Link is Half Duplex!\n");
            }
        } else {
            Console::println("Link is Down!\n");
        }

        Reg(gmac_base, PACKET_FILTER) |= PACKET_FILTER_RECEIVE_ALL | PACKET_FILTER_PROMISCUOUS_MODE;
        Reg(gmac_base, RX_QUEUE_CONTROL0) = RX_QUEUE_CONTROL0_QUEUE0_ENABLE;
        Reg(gmac_base, CONFIGURATION) |= CONFIGURATION_RECEIVER_ENABLE | CONFIGURATION_TRANSMITTER_ENABLE;

        TraceOut();
    }

    static void address(unsigned long value) {
        Reg(gmac_base, ADDRESS0_LOW) = static_cast<unsigned int>(value);
        Reg(gmac_base, ADDRESS0_HIGH) = static_cast<unsigned int>(value >> 32);
    }

    static unsigned long address() {
        return static_cast<unsigned long>(Reg(gmac_base, ADDRESS0_HIGH) & 0xFFFF) << 32 |
               static_cast<unsigned long>(Reg(gmac_base, ADDRESS0_LOW));
    }
};

class MTL {
    enum Bit {
        TX_QUEUE0_OPERATION_MODE_QUEUE0_ENABLE = 2 << 2,
        TX_QUEUE0_OPERATION_MODE_TSF = 2,
        //         RX_QUEUE_DMA_MAP0 = 0,
        //         RX_QUEUE_DMA_MAP1 = 0,
        //         RX_QUEUE0_OPERATION_MODE_RSF = 1 << 5,
    };
    //
    enum Register {
        TX_QUEUE0_OPERATION_MODE = 0xd00,
        //         RX_QUEUE0_DMA_MAP = 0xc30,
        //         RX_QUEUE0_DEBUG = 0xd38,
        //         RX_QUEUE0_OPERATION_MODE = 0xd30,
    };
    //
  public:
    static void init() {
        Reg(gmac_base, TX_QUEUE0_OPERATION_MODE) =
            TX_QUEUE0_OPERATION_MODE_QUEUE0_ENABLE | TX_QUEUE0_OPERATION_MODE_TSF;
        //         Reg(gmac_base, RX_QUEUE0_OPERATION_MODE) |= RX_QUEUE0_OPERATION_MODE_RSF;
        //         Reg(gmac_base, RX_QUEUE_DMA_MAP0) = 0;
        //         Reg(gmac_base, RX_QUEUE_DMA_MAP1) = 0;
        //
        //         while (1) {
        //             Console::println("%d\n", (Reg(gmac_base, RX_QUEUE0_DEBUG) >> 4) & 0x3);
    }
    //     }
};

class Ethernet {
  public:
    static void init() {
        TraceIn();
        Clock::init();
        DMA::reset();
        PHY::init();
        MAC::init();
        MTL::init();
        DMA::init();
        DMA::debug();

        // DMA::receive();
        //  while (1)
        //      Console::println("%d\n", MMC::received());
        //   MTL::init();
        //   MAC::enable();
        //   DMA::receive();
        TraceOut();
    }
};
