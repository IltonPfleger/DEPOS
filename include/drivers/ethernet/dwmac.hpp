#include <utils/Debug.hpp>

class Ethernet {
    static volatile inline unsigned int *base = reinterpret_cast<unsigned int *>(0x16030000);

    static volatile unsigned int &reg(int offsset) {
        return *reinterpret_cast<volatile unsigned int *>(base + offsset);
    }

    struct DMADescriptor {
        uint32_t des0;
        uint32_t des1;
        uint32_t des2;
        uint32_t des3;
    };

    enum Bits {
        DMA_SOFTWARE_RESET = 0x1,
    };

    enum Registers {
        DMA_MODE = 0x1000,
        DMA_CHANNEL_0_TX_CONTROL = 0x1104,
        DMA_CHANNEL_0_TX_DESCRIPTORS_LIST_HADDR = 0x1110,
        DMA_CHANNEL_0_TX_DESCRIPTORS_LIST_ADDR = 0x1114,
        DMA_CHANNEL_0_RX_DESCRIPTORS_LIST_HADDR = 0x1118,
        DMA_CHANNEL_0_RX_DESCRIPTORS_LIST_ADDR = 0x111c,
        DMA_CHANNEL_0_TX_DESCRIPTOR_RING_LENGTH = 0x112c,
        DMA_CHANNEL_0_RX_DESCRIPTOR_RING_LENGTH = 0x1130,
    };

  public:
    static void reset() {
        TraceIn();
        reg(DMA_MODE) |= DMA_SOFTWARE_RESET;
        while (reg(DMA_MODE) & DMA_SOFTWARE_RESET)
            ;
        // memset(s_tx_descriptors, 0, k_number_of_descriptors * sizeof(DMADescriptor));
        // memset(s_rx_descriptors, 0, k_number_of_descriptors * sizeof(DMADescriptor));
        TraceOut();
    }
    static void descriptors() {}

    static void init() {
        TraceIn();
        reset();
        TraceOut();
    }

  private:
    static constexpr unsigned long k_number_of_descriptors = 5;
    static inline DMADescriptor s_tx_descriptors[k_number_of_descriptors];
    static inline DMADescriptor s_rx_descriptors[k_number_of_descriptors];
};
