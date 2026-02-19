#include <abstractions/CPU.hpp>
#include <drivers/Driver.hpp>
#include <machine/Machine.hpp>
#include <memory/Heap.hpp>
#include <network/GenericAddress.hpp>
#include <utils/Debug.hpp>
#include <utils/Guard.hpp>
#include <utils/Observer.hpp>
#include <utils/string.hpp>

// template <typename> class DWC_Ether_QoS;

template <unsigned long Base> class _MDIO : Driver {
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
    using MDIO = _MDIO<Base>;

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
        STATUS_SPEED_MASK = 3 << 14,
        STATUS_SPEED_1000 = 2 << 14,
        STATUS_SPEED_100 = 1 << 14,
        STATUS_SPEED_10 = 0,
        STATUS_FULL_DUPLEX = 1 << 13,
        STATUS_LINK = 1 << 10,
        BASIC_STATUS_AUTO_NEGOTIATION_COMPLETE = 1 << 5,
        RGMII_CONFIG1_TX_CLK_SEL = 1 << 14,
        CHIP_CONFIG_RXC_DELAY_ENABLE = 1 << 8,
        CHIP_CONFIG_SOFTWARE_RESET = 1 << 15,
    };

  public:
    static void init() {
        TraceIn();

        MDIO::clear45(phy, CHIP_CONFIG, CHIP_CONFIG_SOFTWARE_RESET);
        while (!(MDIO::read45(phy, CHIP_CONFIG) & CHIP_CONFIG_SOFTWARE_RESET))
            ;

        MDIO::set(phy, BASIC_CONTROL, BASIC_CONTROL_AUTO_NEGOTIATION_ENABLE | BASIC_CONTROL_RE_AUTO_NEGOTIATION);
        while (!(MDIO::read(phy, BASIC_STATUS) & BASIC_STATUS_AUTO_NEGOTIATION_COMPLETE))
            ;
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

  private:
    static constexpr unsigned int phy = 0;
};

template <unsigned long Base> class DWC_Ether_QoS_MAC : Driver {

    enum Bit {
        PACKET_FILTER_RECEIVE_ALL = 1 << 31,
        PACKET_FILTER_PROMISCUOUS_MODE = 1,
        CONFIGURATION_TRANSMITTER_ENABLE = 1 << 1,
        CONFIGURATION_RECEIVER_ENABLE = 1,
        RX_QUEUE_CONTROL0_QUEUE0_ENABLE = 2,
        DWC_Ether_QoS_PHY_CONTROL_STATUS_LINK_STATUS_UP = 1 << 19,
        DWC_Ether_QoS_PHY_CONTROL_STATUS_LINK_MODE_FULL_DUPLEX = 1 << 16,
        CONFIGURATION_CST = 1 << 21,
        CONFIGURATION_PS = 1 << 15,
        CONFIGURATION_FES = 1 << 14,
        CONFIGURATION_FULL_DUPLEX = 1 << 13,

    };

    enum Register {
        CONFIGURATION = 0x0,
        PACKET_FILTER = 0x8,
        RX_QUEUE_CONTROL0 = 0xa0,
        DWC_Ether_QoS_PHY_CONTROL_STATUS = 0xf8,
        ADDRESS0_LOW = 0x304,
        ADDRESS0_HIGH = 0x300,

    };

  public:
    static void init() {
        TraceIn();
        Reg32(Base, PACKET_FILTER) |= PACKET_FILTER_RECEIVE_ALL | PACKET_FILTER_PROMISCUOUS_MODE;
        Reg32(Base, RX_QUEUE_CONTROL0) = RX_QUEUE_CONTROL0_QUEUE0_ENABLE;
        Reg32(Base, CONFIGURATION) |= CONFIGURATION_RECEIVER_ENABLE | CONFIGURATION_TRANSMITTER_ENABLE;
        Reg32(Base, CONFIGURATION) |= CONFIGURATION_CST | CONFIGURATION_FULL_DUPLEX;
        Console::cout << Console::hex << Reg32(Base, CONFIGURATION) << Console::endl;
        TraceOut();
    }
};

template <typename MyTraits> class DWC_Ether_QoS_DMA : public Observed<>, Driver {
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

        void buffer(void *pointer) {
            uintptr_t addr = reinterpret_cast<uintptr_t>(pointer);
            des0 = static_cast<unsigned int>(addr & 0xFFFFFFFF);
            des1 = static_cast<unsigned int>(addr >> 32);
        }
    };

    struct ReceiveBuffer {

        ReceiveBuffer(Descriptor *d) : m_descriptor(d) { unlock(false); }

        bool lock(bool own) {
            CacheController::flush(m_descriptor, sizeof(Descriptor));
            if (!own || !(m_descriptor->des3 & Descriptor::OWN)) {
                if (!CPU::Atomic::tsl(m_lock)) {
                    return false;
                }
            };
            return true;
        }

        void unlock(bool tail) {
            CPU::Atomic::store(m_lock, 0);
            m_descriptor->buffer(m_data);
            m_descriptor->des2 = 0;
            m_descriptor->des3 = Descriptor::OWN | Descriptor::IOC | Descriptor::VALID;
            CacheController::flush(m_descriptor, sizeof(Descriptor));
            if (tail) Reg32(Base, CH0_RX_DESCRIPTORS_LIST_TAIL_POINTER) = reinterpret_cast<unsigned long>(m_descriptor);
        }

        bool lock() { return lock(true); }
        void unlock() { unlock(true); }

        uint8_t *data() { return m_data; }
        size_t length() { return m_descriptor->des3 & 0x3FFF; }

      private:
        Descriptor *m_descriptor;
        uint8_t m_data[2048];
        bool m_lock;
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
        CH0_INTERRUPT_ENABLE_RBUE = 1 << 7,

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

    static void interrupt(unsigned int) { s_instance->interrupt(); }

    void interrupt() {
        volatile unsigned int &status = Reg32(Base, CH0_INTERRUPT_STATUS);

        if (status & CH0_INTERRUPT_STATUS_RI) {
            status |= CH0_INTERRUPT_STATUS_RI;
            this->notify();
        }

        if (status & CH0_INTERRUPT_STATUS_RBU) {
            Descriptor *current = reinterpret_cast<Descriptor *>(Reg32(Base, CH0_CURRENT_APP_RX_DESCRIPTOR));
            unsigned int i = reinterpret_cast<long>(current - m_rx_descriptors) / sizeof(Descriptor);
            if (!m_rx_buffers[i]->lock(false)) {
                new (m_rx_buffers[i]) ReceiveBuffer(current);
                status |= CH0_INTERRUPT_STATUS_RBU;
            }
        }
    }

    DWC_Ether_QoS_DMA() {
        TraceIn();

        s_instance = this;

        Reg32(Base, SYSBUS_MODE) |= 1 << 11;

        memset(m_tx_descriptors, 0, k_number_of_descriptors * sizeof(Descriptor));
        CacheController::flush(m_tx_descriptors, sizeof(Descriptor) * k_number_of_descriptors);

        for (unsigned int i = 0; i < k_number_of_descriptors; i++) {
            auto &d = m_rx_descriptors[i];
            m_rx_buffers[i] = new ReceiveBuffer(&d);
        }

        Reg32(Base, CH0_RX_DESCRIPTORS_LIST_ADDR) = reinterpret_cast<unsigned long>(m_rx_descriptors) & 0xFFFFFFFF;
        Reg32(Base, CH0_RX_DESCRIPTORS_LIST_HADDR) = reinterpret_cast<unsigned long>(m_rx_descriptors) >> 32;
        Reg32(Base, CH0_RX_DESCRIPTORS_RING_LENGTH) = k_number_of_descriptors - 1;
        Reg32(Base, CH0_INTERRUPT_ENABLE) |= CH0_INTERRUPT_ENABLE_NIE | CH0_INTERRUPT_ENABLE_AIE;
        Reg32(Base, CH0_INTERRUPT_ENABLE) |= CH0_INTERRUPT_ENABLE_RIE | CH0_INTERRUPT_ENABLE_RBUE;

        Reg32(Base, CH0_TX_DESCRIPTORS_LIST_ADDR) = reinterpret_cast<unsigned long>(m_tx_descriptors) & 0xFFFFFFFF;
        Reg32(Base, CH0_TX_DESCRIPTORS_LIST_HADDR) = reinterpret_cast<unsigned long>(m_tx_descriptors) >> 32;
        Reg32(Base, CH0_TX_DESCRIPTORS_RING_LENGTH) = k_number_of_descriptors - 1;

        for (auto i : MyTraits::IRQs)
            IC::bind(i, interrupt);

        Reg32(Base, CH0_TX_CONTROL) |= 1;
        Reg32(Base, CH0_RX_CONTROL) |= 1;

        Reg32(Base, CH0_RX_DESCRIPTORS_LIST_TAIL_POINTER) =
            reinterpret_cast<unsigned long>(m_rx_descriptors + k_number_of_descriptors);

        TraceOut();
    }

    auto receive() {
        unsigned int i = 0;

        while (1) {
            if (!m_rx_buffers[i]->lock()) break;
            i = (i + 1) % k_number_of_descriptors;
        }

        CacheController::flush(&m_rx_buffers[i], sizeof(m_rx_buffers[i]));

        return Guard<ReceiveBuffer, static_cast<bool (ReceiveBuffer::*)()>(&ReceiveBuffer::lock),
                     static_cast<void (ReceiveBuffer::*)()>(&ReceiveBuffer::unlock)>(m_rx_buffers[i], false);
    }

    // Broken For Multicore
    int send(void *frame, unsigned int length) {
        CacheController::flush(frame, length);

        Descriptor &d = *reinterpret_cast<Descriptor *>(Reg32(Base, CH0_CURRENT_APP_TX_DESCRIPTOR));

        d.buffer(frame);
        d.des3 = Descriptor::OWN | Descriptor::FIRST | Descriptor::LAST | (length & 0x3FFF);
        d.des2 = (length & 0x7FFF);
        CacheController::flush(&d, sizeof(Descriptor));
        Reg32(Base, CH0_TX_DESCRIPTORS_LIST_TAIL_POINTER) = reinterpret_cast<unsigned long>(&d);

        while (1) {
            CacheController::flush(&d, sizeof(Descriptor));
            if (!(d.des3 & Descriptor::OWN)) {
                if (d.des3 & Descriptor::TX_ERROR) return 0;
                return length;
            }
        }
    }

  private:
    static constexpr unsigned long Base = MyTraits::Address;
    static constexpr unsigned int k_number_of_descriptors = 10;
    static inline DWC_Ether_QoS_DMA *s_instance;
    ReceiveBuffer *m_rx_buffers[k_number_of_descriptors];
    Descriptor m_tx_descriptors[k_number_of_descriptors];
    Descriptor m_rx_descriptors[k_number_of_descriptors];
};

template <unsigned long Base> class DWC_Ether_QoS_MTL : Driver {
    enum Bit {
        TX_QUEUE0_OPERATION_MODE_TSF = 2,
        RX_QUEUE0_OPERATION_MODE_RSF = 1 << 5,
        RX_QUEUE0_OPERATION_MODE_FEP = 1 << 4,
        RX_QUEUE0_OPERATION_MODE_FUP = 1 << 3,

    };

    enum Register {
        TX_QUEUE0_OPERATION_MODE = 0xd00,
        RX_QUEUE0_OPERATION_MODE = 0xd30,

    };

  public:
    static void init() {
        TraceIn();
        Reg32(Base, TX_QUEUE0_OPERATION_MODE) |= TX_QUEUE0_OPERATION_MODE_TSF;
        Reg32(Base, RX_QUEUE0_OPERATION_MODE) |= RX_QUEUE0_OPERATION_MODE_RSF;
        TraceOut();
    }
};

template <typename MyTraits> class DWC_Ether_QoS : public DWC_Ether_QoS_DMA<MyTraits> {
    using DMA = DWC_Ether_QoS_DMA<MyTraits>;
    using MTL = DWC_Ether_QoS_MTL<MyTraits::Address>;
    using PHY = DWC_Ether_QoS_PHY<MyTraits::Address>;
    using MAC = DWC_Ether_QoS_MAC<MyTraits::Address>;

  public:
    DWC_Ether_QoS() : DMA() {}

    static auto *instance() { return m_device; }

    static void init() {
        if (!m_device) {
            TraceIn();
            DMA::reset();
            MTL::init();
            PHY::init();
            MAC::init();
            m_device = new (Heap::SYSTEM) DWC_Ether_QoS();
            TraceOut();
        }
    }

    auto mac() { return GenericAddress<6>(MyTraits::MAC); }
    auto ip() { return GenericAddress<4>(MyTraits::IP); }

  private:
    static inline DWC_Ether_QoS *m_device;
};
