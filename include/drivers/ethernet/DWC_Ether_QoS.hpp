#include <abstractions/CPU.hpp>
#include <drivers/Driver.hpp>
#include <machine/Machine.hpp>
#include <memory/Heap.hpp>
#include <network/GenericAddress.hpp>
#include <utils/Debug.hpp>
#include <utils/string.hpp>

template <unsigned long Base> class DWC_Ether_QoS_MDIO : Driver {
    enum Register { BASE = 0x200, DATA = 0x204 };
    enum Bit {
        CLOCK_250_300 = 0x5 << 8,
        WRITE = 0x1 << 2,
        READ = 0x3 << 2,
        BUSY = 1,
    };

  public:
    static void wait() {
        while (Reg32(Base, BASE) & BUSY)
            ;
    }

    static void set(unsigned char phy, unsigned char dev, unsigned short data) { write(phy, dev, read(phy, dev) | data); }

    static void clear(unsigned char phy, unsigned char dev, unsigned short data) { write(phy, dev, read(phy, dev) & ~data); }

    static void write(unsigned char phy, unsigned char dev, unsigned short data) {
        Reg32(Base, DATA) = data;
        Reg32(Base, BASE) = BUSY | WRITE | CLOCK_250_300 | ((phy & 0x1F) << 21) | ((dev & 0x1F) << 16);
        wait();
    }
    static unsigned short read(unsigned char phy, unsigned char dev) {
        Reg32(Base, BASE) = BUSY | READ | CLOCK_250_300 | ((phy & 0x1F) << 21) | ((dev & 0x1F) << 16);
        wait();
        return Reg32(Base, DATA) & 0xFFFF;
    }

    static void write45(unsigned char phy, unsigned short reg, unsigned short data) {
        write(phy, 0x1E, reg);
        write(phy, 0x1F, data);
    }

    static void set45(unsigned char phy, unsigned short reg, unsigned short data) {
        write45(phy, reg, read45(phy, reg) | data);
    }

    static void clear45(unsigned char phy, unsigned short reg, unsigned short data) {
        write45(phy, reg, read45(phy, reg) & ~data);
    }

    static unsigned short read45(unsigned char phy, unsigned short reg) { return write(phy, 0x1E, reg), read(phy, 0x1F); }
};

template <unsigned long Base> class DWC_Ether_QoS_PHY {
    using MDIO = DWC_Ether_QoS_MDIO<Base>;

    enum Register {
        BASIC_CONTROL = 0x0,
        BASIC_STATUS = 0x1,
        CHIP_CONFIG = 0xA001,
        RGMII_CONFIG1 = 0xA003,
        PAD_DRIVE_STRENGTH_CFG = 0xA010,
        SYNC_E_CFG = 0xA012,
        STATUS = 0x11,
        DWC_Ether_QoS_PHY_ID_1 = 0x2,
        DWC_Ether_QoS_PHY_ID_2 = 0x3,
    };
    enum Bit {
        BASIC_CONTROL_RESET = 1 << 15,
        BASIC_CONTROL_AUTO_NEGOTIATION_ENABLE = 1 << 12,
        BASIC_CONTROL_RE_AUTO_NEGOTIATION = 1 << 9,
        BASIC_STATUS_AUTO_NEGOTIATION_COMPLETE = 1 << 5,
        CHIP_CONFIG_SOFTWARE_RESET = 1 << 15,
    };

  public:
    static void init() {
        TraceIn();

        MDIO::clear45(phy, CHIP_CONFIG, CHIP_CONFIG_SOFTWARE_RESET);
        while (!(MDIO::read45(phy, CHIP_CONFIG) & CHIP_CONFIG_SOFTWARE_RESET))
            ;

        // rgmii_sw_dr_2 = <0x0>;
        MDIO::clear45(phy, PAD_DRIVE_STRENGTH_CFG, 1 << 12);

        // rgmii_sw_dr = <0x3>;
        MDIO::set45(phy, PAD_DRIVE_STRENGTH_CFG, 3 << 4);

        // rgmii_sw_dr_rxc = <0x6>;
        MDIO::clear45(phy, PAD_DRIVE_STRENGTH_CFG, 7 << 13);
        MDIO::set45(phy, PAD_DRIVE_STRENGTH_CFG, 6 << 13);

        // rxc_dly_en = <0>;
        MDIO::clear45(phy, CHIP_CONFIG, 1 << 8);

        // rx_delay_sel = <0xa>;
        MDIO::clear45(phy, RGMII_CONFIG1, 0xF << 10);
        MDIO::set45(phy, RGMII_CONFIG1, 0xa << 10);

        // tx_delay_sel_fe = <5>;
        // MDIO::clear45(phy, RGMII_CONFIG1, 0xF << 4);
        // MDIO::set45(phy, RGMII_CONFIG1, 5 << 4);

        // tx_delay_sel = <0xa>;
        MDIO::clear45(phy, RGMII_CONFIG1, 0xF);
        MDIO::set45(phy, RGMII_CONFIG1, 0xa);

        // tx_inverted_1000 = <0x1>;
        MDIO::set45(phy, RGMII_CONFIG1, 1 << 14);

        MDIO::set(phy, BASIC_CONTROL, BASIC_CONTROL_AUTO_NEGOTIATION_ENABLE | BASIC_CONTROL_RE_AUTO_NEGOTIATION);
        while (!(MDIO::read(phy, BASIC_STATUS) & (BASIC_STATUS_AUTO_NEGOTIATION_COMPLETE)))
            ;
    }

  private:
    static constexpr unsigned int phy = 0;
};

template <unsigned long Base> class DWC_Ether_QoS_MAC : Driver {
    enum Bits {
        PACKET_FILTER_RECEIVE_ALL = 1 << 31,
        PACKET_FILTER_PROMISCUOUS_MODE = 1,
        CONFIGURATION_TRANSMITTER_ENABLE = 1 << 1,
        CONFIGURATION_RECEIVER_ENABLE = 1,
        RX_QUEUE_CONTROL0_QUEUE0_ENABLE = 2,
        CONFIGURATION_CST = 1 << 21,
        CONFIGURATION_DM = 1 << 13,

    };

    enum Registers {
        CONFIGURATION = 0x0,
        PACKET_FILTER = 0x8,
        RX_QUEUE_CONTROL0 = 0xa0,
        ADDRESS0_LOW = 0x304,
        ADDRESS0_HIGH = 0x300,

    };

  public:
    static void init() {
        TraceIn();
        Reg32(Base, PACKET_FILTER) |= PACKET_FILTER_RECEIVE_ALL | PACKET_FILTER_PROMISCUOUS_MODE;
        Reg32(Base, RX_QUEUE_CONTROL0) = RX_QUEUE_CONTROL0_QUEUE0_ENABLE;
        Reg32(Base, CONFIGURATION) |= CONFIGURATION_RECEIVER_ENABLE | CONFIGURATION_TRANSMITTER_ENABLE;
        Reg32(Base, CONFIGURATION) |= CONFIGURATION_CST | CONFIGURATION_DM;
        TraceOut();
    }
};

template <typename MyTraits> class DWC_Ether_QoS_DMA {

    struct Descriptor {
        uint32_t des0;
        uint32_t des1;
        unsigned int des2;
        unsigned int des3;

        enum {
            OWN = 1 << 31,
            IOC = 1 << 30,
            FD = 1 << 29,
            LD = 1 << 28,
            ES = 1 << 15,
            BUF1V = 1 << 24,
        };

        uint64_t &buffer() { return *reinterpret_cast<uint64_t *>(&des0); }
    };

    struct Buffer {
        friend DWC_Ether_QoS_DMA;

        auto data() { return m_data; }
        auto length() { return m_descriptor->des3 & 0x3FFF; }

      private:
        Descriptor *m_descriptor;
        unsigned char m_data[1518];
        bool m_locked;
    };

    struct Registers {
        uint32_t mode;
        // 0x1000
        uint32_t sysbus_mode;
        // 0x1004
        uint32_t _1008[(0x1100 - 0x1008) / 4]; // 0x1008
        uint32_t ch0_control;
        // 0x1100
        uint32_t ch0_tx_control;
        // 0x1104
        uint32_t ch0_rx_control;
        // 0x1108
        uint32_t _110c;
        // 0x110c
        uint32_t ch0_txdesc_list_haddress;
        // 0x1110
        uint32_t ch0_txdesc_list_address;
        // 0x1114
        uint32_t ch0_rxdesc_list_haddress;
        // 0x1118
        uint32_t ch0_rxdesc_list_address;
        // 0x111c
        uint32_t ch0_tx_tail_pointer;
        // 0x1120
        uint32_t _1124;
        // 0x1124
        uint32_t ch0_rx_tail_pointer;
        // 0x1128
        uint32_t ch0_txdesc_ring_length;
        // 0x112c
        uint32_t ch0_rxdesc_ring_length;
        // 0x1130
        uint32_t interrupt_enable;
        // 0x1134
        uint32_t _1138[(0x114c - 0x1138) / 4]; // 0x1138
        uint32_t current_rx_descriptor;
        // 0x114c
        uint32_t _1150[(0x1160 - 0x1150) / 4]; // 0x1150
        uint32_t interrupt_status;
        // 0x1160
    };

    enum Bits {
        MODE_SOFTWARE_RESET = 1 << 0,
        SYSBUS_MODE_EAME = 1 << 11,
        SYSBUS_MODE_FB = 1 << 0,
        SYSBUS_MODE_BLEN4 = 1 << 1,
        INTERRUPT_ENABLE_NIE = 1 << 15,
        INTERRUPT_ENABLE_AIE = 1 << 14,
        INTERRUPT_ENABLE_RIE = 1 << 6,
        INTERRUPT_ENABLE_RBUE = 1 << 7,
        INTERRUPT_STATUS_RI = 1 << 6,
        INTERRUPT_STATUS_RBU = 1 << 7,
    };

  public:
    DWC_Ether_QoS_DMA() {
        TraceIn();

        s_instance = this;
        m_tx_head = 0;
        m_rx_head = 0;
        m_registers = reinterpret_cast<volatile Registers *>(MyTraits::Address + 0x1000);

        memset(m_tx_descriptors, 0, k_number * sizeof(Descriptor));
        CacheController::flush(m_tx_descriptors, k_number * sizeof(Descriptor));

        for (size_t i = 0; i < k_number; i++) {
            m_rx_buffers[i].m_descriptor = &m_rx_descriptors[i];
            m_rx_descriptors[i].buffer() = reinterpret_cast<uintptr_t>(m_rx_buffers[i].m_data);
            m_rx_descriptors[i].des3 = Descriptor::OWN | Descriptor::IOC | Descriptor::BUF1V;
            CacheController::flush(&m_rx_descriptors[i], sizeof(Descriptor));
            m_tx_buffers[i].m_locked = false;
        }

        m_registers->sysbus_mode |= SYSBUS_MODE_EAME;

        m_registers->ch0_rxdesc_list_address = reinterpret_cast<uintptr_t>(m_rx_descriptors) & 0xFFFFFFFF;
        m_registers->ch0_rxdesc_list_haddress = reinterpret_cast<uintptr_t>(m_rx_descriptors) >> 32;
        m_registers->ch0_rxdesc_ring_length = k_number - 1;

        m_registers->ch0_txdesc_list_address = reinterpret_cast<uintptr_t>(m_tx_descriptors) & 0xFFFFFFFF;
        m_registers->ch0_txdesc_list_haddress = reinterpret_cast<uintptr_t>(m_tx_descriptors) >> 32;
        m_registers->ch0_txdesc_ring_length = k_number - 1;

        m_registers->ch0_tx_control |= 1;
        m_registers->ch0_rx_control |= 1;

        // m_registers->interrupt_enable = INTERRUPT_ENABLE_NIE | INTERRUPT_ENABLE_AIE;
        // m_registers->interrupt_enable |= INTERRUPT_ENABLE_RBUE;
        // m_registers->interrupt_enable |= INTERRUPT_ENABLE_RIE;

        for (auto i : MyTraits::IRQs)
            IC::bind(i, interrupt);

        m_registers->ch0_rx_tail_pointer = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(m_rx_descriptors + k_number));

        TraceOut();
    }

    static void interrupt(unsigned int) { s_instance->interrupt(); }

    void interrupt() {
        volatile unsigned int &status = m_registers->interrupt_status;
        unsigned int done = 0;

        // TraceIn();

        // if (status & INTERRUPT_STATUS_RI) {
        // done |= INTERRUPT_STATUS_RI;
        // }

        // if (status & INTERRUPT_STATUS_RBU) {
        // Descriptor *current = reinterpret_cast<Descriptor *>(m_registers->current_rx_descriptor);
        // unsigned int i = reinterpret_cast<long>(current - m_rx_descriptors);
        // if (!m_rx_buffers[i].m_locked) {
        // free(&m_rx_buffers[i]);
        //}
        // done |= INTERRUPT_STATUS_RBU;
        // }

        status |= done;
    }

    static void reset() {
        TraceIn();
        auto &reg = *reinterpret_cast<volatile unsigned int *>(MyTraits::Address + 0x1000);
        reg |= MODE_SOFTWARE_RESET;
        while (reg & MODE_SOFTWARE_RESET)
            ;
        TraceOut();
    }

    // Buffer *alloc() {
    // for (size_t i = 0; i < k_number; i++) {
    // size_t i = (m_tx_head + i) % k_number;
    // if (!CPU::Atomic::tsl(m_tx_buffers[i].m_locked)) {
    // return &m_tx_buffers[i];
    //}
    //}
    // return nullptr;
    // }

    void free(Buffer *b) {
        b->m_locked = false;
        b->m_descriptor->buffer() = reinterpret_cast<uintptr_t>(b->m_data);
        b->m_descriptor->des3 = Descriptor::OWN | Descriptor::IOC | Descriptor::BUF1V;
        b->m_descriptor->des2 = 0;
        CacheController::flush(b->m_descriptor, sizeof(Descriptor));
        m_registers->ch0_rx_tail_pointer = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(b->m_descriptor));
    }

    int send(const void *p, size_t s) {
        Descriptor &d = m_tx_descriptors[m_tx_head];

        d.buffer() = reinterpret_cast<uint64_t>(p);
        d.des3 = Descriptor::OWN | Descriptor::FD | Descriptor::LD | (s & 0x3FFF);
        d.des2 = s & 0x3FFF;

        CacheController::flush(&d, sizeof(Descriptor));
        CacheController::flush(p, s);

        m_tx_head = (m_tx_head + 1) % k_number;
        m_registers->ch0_tx_tail_pointer = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(m_tx_descriptors + m_tx_head));

        while (1) {
            if (!(d.des3 & Descriptor::OWN)) {
                if (d.des3 & Descriptor::ES) break;
                return s;
            }
            CacheController::flush(&d, sizeof(Descriptor));
        }
        return 0;
    }

    Buffer *receive() {
        unsigned int i = m_rx_head;

        Descriptor *descriptor = &m_rx_descriptors[i];

        CacheController::flush(descriptor, sizeof(Descriptor));

        if (descriptor->des3 & Descriptor::OWN) {
            return nullptr;
        }

        m_rx_head = (m_rx_head + 1) % k_number;

        CacheController::flush(&m_rx_buffers[i], sizeof(Buffer));

        m_rx_buffers[i].m_locked = true;

        return &m_rx_buffers[i];
    }

  private:
    static inline DWC_Ether_QoS_DMA *s_instance;
    static constexpr size_t k_number = 10;

  private:
    volatile Registers *m_registers;
    Descriptor m_tx_descriptors[k_number];
    Descriptor m_rx_descriptors[k_number];
    Buffer m_rx_buffers[k_number];
    Buffer m_tx_buffers[k_number];
    unsigned int m_tx_head;
    unsigned int m_rx_head;
};

template <unsigned long Base> class DWC_Ether_QoS_MTL : Driver {
    enum Bits {
        TX_QUEUE0_OPERATION_MODE_TSF = 1 << 1,
        TX_QUEUE0_OPERATION_MODE_ENABLE = 1 << 3,
        RX_QUEUE0_OPERATION_MODE_RSF = 1 << 5,
        RX_QUEUE0_OPERATION_MODE_FEP = 1 << 4,
        RX_QUEUE0_OPERATION_MODE_FUP = 1 << 3,

    };

    enum Registers {
        TX_QUEUE0_OPERATION_MODE = 0xd00,
        RX_QUEUE0_OPERATION_MODE = 0xd30,

    };

  public:
    static void init() {
        TraceIn();
        Reg32(Base, TX_QUEUE0_OPERATION_MODE) |= TX_QUEUE0_OPERATION_MODE_TSF | TX_QUEUE0_OPERATION_MODE_ENABLE;
        Reg32(Base, RX_QUEUE0_OPERATION_MODE) |= RX_QUEUE0_OPERATION_MODE_FUP | RX_QUEUE0_OPERATION_MODE_RSF;
        TraceOut();
    }
};

template <typename MyTraits> class DWC_Ether_QoS : public DWC_Ether_QoS_DMA<MyTraits> {
    using DMA = DWC_Ether_QoS_DMA<MyTraits>;
    using MTL = DWC_Ether_QoS_MTL<MyTraits::Address>;
    using PHY = DWC_Ether_QoS_PHY<MyTraits::Address>;
    using MAC = DWC_Ether_QoS_MAC<MyTraits::Address>;

    DWC_Ether_QoS() : DMA() {}

  public:
    static void init() {
        TraceIn();
        DMA::reset();
        PHY::init();
        MTL::init();
        s_instance = new (Heap::SYSTEM) DWC_Ether_QoS();
        MAC::init();
        volatile unsigned int i = 5'000'000;
        while (i)
            i = i - 1;

        TraceOut();
    }

    static bool running() { return s_instance ? true : false; }

    static auto *instance() {
        ERROR(!s_instance);
        return s_instance;
    }

    auto mac() { return GenericAddress<6>(MyTraits::MAC); }
    auto ip() { return GenericAddress<4>(MyTraits::IP); }

  private:
    static inline DWC_Ether_QoS *s_instance;
};
