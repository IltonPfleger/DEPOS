#include <Alarm.hpp>
#include <architecture/CPU.hpp>
#include <drivers/Driver.hpp>
#include <machine/Machine.hpp>
#include <memory/Heap.hpp>
#include <network/GenericAddress.hpp>
#include <network/ethernet/Ethernet.hpp>
#include <utils/Debug.hpp>
#include <utils/string.hpp>

namespace DEPOS {

template <unsigned long Base> class DWC_Ether_QoS_MDIO : Driver {
    enum Register { BASE = 0x200, DATA = 0x204 };
    enum Bit {
        CLOCK_250_300 = 0x5 << 8,
        WRITE         = 0x1 << 2,
        READ          = 0x3 << 2,
        BUSY          = 1,
    };

  public:
    static void wait() {
        while (Reg32(Base, BASE) & BUSY)
            ;
    }

    static void set(unsigned char phy, unsigned char dev, unsigned short data) {
        write(phy, dev, read(phy, dev) | data);
    }

    static void clear(unsigned char phy, unsigned char dev, unsigned short data) {
        write(phy, dev, read(phy, dev) & ~data);
    }

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

    static unsigned short read45(unsigned char phy, unsigned short reg) {
        return write(phy, 0x1E, reg), read(phy, 0x1F);
    }
};

template <unsigned long Base> class DWC_Ether_QoS_PHY {
    using MDIO = DWC_Ether_QoS_MDIO<Base>;

    enum Register {
        BASIC_CONTROL          = 0x0,
        BASIC_STATUS           = 0x1,
        CHIP_CONFIG            = 0xA001,
        RGMII_CONFIG1          = 0xA003,
        PAD_DRIVE_STRENGTH_CFG = 0xA010,
        SYNC_E_CFG             = 0xA012,
        STATUS                 = 0x11,
        DWC_Ether_QoS_PHY_ID_1 = 0x2,
        DWC_Ether_QoS_PHY_ID_2 = 0x3,
    };
    enum Bit {
        BASIC_CONTROL_RESET                    = 1 << 15,
        BASIC_CONTROL_AUTO_NEGOTIATION_ENABLE  = 1 << 12,
        BASIC_CONTROL_RE_AUTO_NEGOTIATION      = 1 << 9,
        BASIC_STATUS_AUTO_NEGOTIATION_COMPLETE = 1 << 5,
        CHIP_CONFIG_SOFTWARE_RESET             = 1 << 15,
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
        PACKET_FILTER_RECEIVE_ALL        = 1 << 31,
        PACKET_FILTER_PROMISCUOUS_MODE   = 1,
        CONFIGURATION_TRANSMITTER_ENABLE = 1 << 1,
        CONFIGURATION_RECEIVER_ENABLE    = 1,
        RX_QUEUE_CONTROL0_QUEUE0_ENABLE  = 2,
        CONFIGURATION_CST                = 1 << 21,
        CONFIGURATION_DM                 = 1 << 13,

    };

    enum Registers {
        CONFIGURATION     = 0x0,
        PACKET_FILTER     = 0x8,
        RX_QUEUE_CONTROL0 = 0xa0,
        ADDRESS0_LOW      = 0x304,
        ADDRESS0_HIGH     = 0x300,

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

template <typename MyTraits> class DWC_Ether_QoS_DMA : public Driver {
    typedef NIC::Buffer Buffer;

    struct Descriptor {
        uint32_t des0;
        uint32_t des1;
        unsigned int des2;
        unsigned int des3;

        enum {
            OWN   = 1 << 31,
            IOC   = 1 << 30,
            FD    = 1 << 29,
            LD    = 1 << 28,
            ES    = 1 << 15,
            BUF1V = 1 << 24,
        };

        auto length() const { return des3 & 0x3FFF; }
        uint64_t buffer() const { return (static_cast<uint64_t>(des1) << 32) | des0; }
        void buffer(uint64_t addr) {
            des0 = static_cast<uint32_t>(addr);
            des1 = static_cast<uint32_t>(addr >> 32);
        }
    };

    enum Registers {
        DMA_MODE               = 0x1000,
        DMA_SYSBUS_MODE        = 0x1004,
        CH0_CONTROL            = 0x1100,
        CH0_TX_CONTROL         = 0x1104,
        CH0_RX_CONTROL         = 0x1108,
        CH0_TXDESC_LIST_HADDR  = 0x1110,
        CH0_TXDESC_LIST_ADDR   = 0x1114,
        CH0_RXDESC_LIST_HADDR  = 0x1118,
        CH0_RXDESC_LIST_ADDR   = 0x111c,
        CH0_TX_TAIL_POINTER    = 0x1120,
        CH0_RX_TAIL_POINTER    = 0x1128,
        CH0_TXDESC_RING_LENGTH = 0x112c,
        CH0_RXDESC_RING_LENGTH = 0x1130,
        CH0_INTERRUPT_ENABLE   = 0x1134,
        CH0_INTERRUPT_STATUS   = 0x1160,
    };

    enum Bits {
        MODE_SOFTWARE_RESET   = 1 << 0,
        SYSBUS_MODE_EAME      = 1 << 11,
        SYSBUS_MODE_FB        = 1 << 0,
        SYSBUS_MODE_BLEN4     = 1 << 1,
        INTERRUPT_ENABLE_NIE  = 1 << 15,
        INTERRUPT_ENABLE_AIE  = 1 << 14,
        INTERRUPT_ENABLE_RIE  = 1 << 6,
        INTERRUPT_ENABLE_RBUE = 1 << 7,
        INTERRUPT_STATUS_RI   = 1 << 6,
        INTERRUPT_STATUS_RBU  = 1 << 7,
    };

  public:
    DWC_Ether_QoS_DMA(NIC *owner)
        : m_tx_head(0),
          m_rx_head(0) {
        TraceIn();

        memset(m_tx_descriptors, 0, Number * sizeof(Descriptor));
        Cache::flush(m_tx_descriptors, Number * sizeof(Descriptor));

        for (size_t i = 0; i < Number; i++) {
            Buffer &buffer         = m_rx_buffers[i];
            Descriptor &descriptor = m_rx_descriptors[i];

            buffer = Buffer(new unsigned char[MTU], MTU, owner);

            descriptor.buffer(reinterpret_cast<uintptr_t>(buffer.data()));
            descriptor.des3 = Descriptor::OWN | Descriptor::IOC | Descriptor::BUF1V;

            Cache::flush(&descriptor, sizeof(Descriptor));
        }

        Reg32(Address, DMA_SYSBUS_MODE) |= SYSBUS_MODE_EAME;

        uintptr_t rx_addr                      = reinterpret_cast<uintptr_t>(m_rx_descriptors);
        Reg32(Address, CH0_RXDESC_LIST_ADDR)   = static_cast<uint32_t>(rx_addr);
        Reg32(Address, CH0_RXDESC_LIST_HADDR)  = static_cast<uint32_t>(rx_addr >> 32);
        Reg32(Address, CH0_RXDESC_RING_LENGTH) = Number - 1;

        uintptr_t tx_addr                      = reinterpret_cast<uintptr_t>(m_tx_descriptors);
        Reg32(Address, CH0_TXDESC_LIST_ADDR)   = static_cast<uint32_t>(tx_addr);
        Reg32(Address, CH0_TXDESC_LIST_HADDR)  = static_cast<uint32_t>(tx_addr >> 32);
        Reg32(Address, CH0_TXDESC_RING_LENGTH) = Number - 1;

        Reg32(Address, CH0_TX_CONTROL) |= 1;
        Reg32(Address, CH0_RX_CONTROL) |= 1;

        Reg32(Address, CH0_RX_TAIL_POINTER) = static_cast<uint32_t>(rx_addr + (Number * sizeof(Descriptor)));

        TraceOut();
    }

    static void reset() {
        Reg32(Address, DMA_MODE) |= MODE_SOFTWARE_RESET;
        while (Reg32(Address, DMA_MODE) & MODE_SOFTWARE_RESET)
            ;
    }

    // static void interrupt(unsigned int) { s_instance->interrupt(); }

    // void interrupt() {
    //     volatile unsigned int &status = m_registers->interrupt_status;
    //     unsigned int done             = 0;

    //    // TraceIn();

    //    // if (status & INTERRUPT_STATUS_RI) {
    //    // done |= INTERRUPT_STATUS_RI;
    //    // }

    //    // if (status & INTERRUPT_STATUS_RBU) {
    //    // Descriptor *current = reinterpret_cast<Descriptor
    //    // *>(m_registers->current_rx_descriptor); unsigned int i =
    //    // reinterpret_cast<long>(current - m_rx_descriptors); if
    //    // (!m_rx_buffers[i].m_locked) { free(&m_rx_buffers[i]);
    //    //}
    //    // done |= INTERRUPT_STATUS_RBU;
    //    // }

    //    status |= done;
    //}

    void release(Buffer *b) {
        Descriptor &d = m_rx_descriptors[b->id()];
        d.buffer(reinterpret_cast<uintptr_t>(b->data()));
        d.des2 = 0;
        d.des3 = Descriptor::OWN | Descriptor::IOC | Descriptor::BUF1V;
        Cache::flush(&d, sizeof(Descriptor));

        Reg32(Address, CH0_RX_TAIL_POINTER) = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(&d));
    }

    int send(const void *p, size_t s) {
        Descriptor &d = m_tx_descriptors[m_tx_head];

        d.buffer(reinterpret_cast<uint64_t>(p));
        d.des3 = Descriptor::OWN | Descriptor::FD | Descriptor::LD | (s & 0x3FFF);
        d.des2 = s & 0x3FFF;

        Cache::flush(&d, sizeof(Descriptor));
        Cache::flush(p, s);

        m_tx_head = (m_tx_head + 1) % Number;

        Reg32(Address, CH0_TX_TAIL_POINTER) = reinterpret_cast<uintptr_t>(m_tx_descriptors + m_tx_head);

        return 0;
    }

    Buffer *receive() {
        Descriptor *descriptor = &m_rx_descriptors[m_rx_head];
        Buffer *buffer         = &m_rx_buffers[m_rx_head];

        Cache::flush(descriptor, sizeof(Descriptor));

        if (descriptor->des3 & Descriptor::OWN) return nullptr;

        Cache::flush(buffer, sizeof(Buffer));

        buffer->length() = descriptor->length();
        buffer->id()     = m_rx_head;

        m_rx_head = (m_rx_head + 1) % Number;
        return buffer;
    }

  private:
    static inline DWC_Ether_QoS_DMA *s_instance;
    static constexpr uintptr_t Address = MyTraits::Address;
    static constexpr size_t Number     = 10;
    static constexpr size_t MTU        = 1522;

  private:
    Descriptor m_tx_descriptors[Number];
    Descriptor m_rx_descriptors[Number];
    Buffer m_rx_buffers[Number];
    Buffer m_tx_buffers[Number];
    unsigned int m_tx_head;
    unsigned int m_rx_head;
};

template <unsigned long Base> class DWC_Ether_QoS_MTL : Driver {
    enum Bits {
        TX_QUEUE0_OPERATION_MODE_TSF    = 1 << 1,
        TX_QUEUE0_OPERATION_MODE_ENABLE = 1 << 3,
        RX_QUEUE0_OPERATION_MODE_RSF    = 1 << 5,
        RX_QUEUE0_OPERATION_MODE_FEP    = 1 << 4,
        RX_QUEUE0_OPERATION_MODE_FUP    = 1 << 3,

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

template <typename Tag> class DWC_Ether_QoS final : public NIC {
    using MyTraits = Traits<DWC_Ether_QoS<Tag>>;
    using DMA      = DWC_Ether_QoS_DMA<MyTraits>;
    using MTL      = DWC_Ether_QoS_MTL<MyTraits::Address>;
    using PHY      = DWC_Ether_QoS_PHY<MyTraits::Address>;
    using MAC      = DWC_Ether_QoS_MAC<MyTraits::Address>;

    DWC_Ether_QoS() {
        TraceIn();
        DMA::reset();
        PHY::init();
        MTL::init();
        m_dma = new DMA(this);
        MAC::init();
        Alarm::udelay(1'000'000);
        NIC::init();
        TraceOut();
    }

  public:
    int send(const void *d, size_t s) override { return m_dma->send(d, s); }
    Buffer *receive() override { return m_dma->receive(); }
    void free(Buffer *b) override { m_dma->release(b); }

    Ethernet::Address address() { return MyTraits::MAC; }

    static auto *instance() {
        static DWC_Ether_QoS instance;
        return &instance;
    }

  private:
    DMA *m_dma;
};

} // namespace DEPOS
