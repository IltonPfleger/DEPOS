#pragma once

#include <Traits.hpp>
#include <abstractions/VirtualCPU.hpp>
#include <drivers/virtio/Handler.hpp>
#include <memory/Heap.hpp>
#include <network/NetworkAdapter.hpp>
#include <utils/Observer.hpp>

namespace virtio {

struct NetHeader {
    uint8_t flags;
    uint8_t gso_type;
    uint16_t hdr_len;
    uint16_t gso_size;
    uint16_t csum_start;
    uint16_t csum_offset;
    // uint16_t num_buffers; // Apenas se VIRTIO_NET_F_MRG_RXBUF estiver ativo
};

template <typename Device, uintptr_t Base>
class Network : public Handler<Network<Device, Base>>, public Observer<const unsigned char *, size_t> {
  public:
    static Network *instance() {
        if (!s_instance) s_instance = new Network();
        return s_instance;
    }

    void notify(unsigned int source) {
        if (source != k_tx_queue) return;

        VirtQueue &queue = this->m_queues[k_tx_queue];
        int head_id;

        while ((head_id = queue.available()) >= 0) {
            int current_id = head_id;
            uint32_t total_length = 0;
            bool first_descriptor = true;

            while (true) {
                VirtQueue::RingDescriptor *descriptor = queue.get(current_id);
                uint8_t *data = reinterpret_cast<uint8_t *>(descriptor->address);
                uint32_t len = descriptor->length;

                if (first_descriptor) {
                    size_t header_size = sizeof(NetHeader);
                    data += header_size;
                    len -= header_size;
                    first_descriptor = false;
                }

                if (len > 0) {
                    NetworkAdapter<Device>::instance()->send(data, len);
                }

                total_length += descriptor->length;
                if (!(descriptor->flags & 0x1)) break;
                current_id = descriptor->next;
            }

            queue.free(head_id, total_length);
            this->interrupts(0x1);
        }
    }

    void update(const unsigned char *buffer, size_t size) override {
        VirtQueue &queue = this->m_queues[k_rx_queue];
        int id = queue.available();

        if (id < 0) return;

        auto descriptor = queue.get(id);
        auto *destination = reinterpret_cast<uint8_t *>(descriptor->address);

        NetHeader hdr;
        memset(&hdr, 0, sizeof(NetHeader));

        size_t header_size = sizeof(NetHeader);
        memcpy(destination, &hdr, header_size);

        memcpy(destination + header_size, buffer, size);

        descriptor->length = size + header_size;

        queue.free(id, descriptor->length);
        this->interrupts(0x1);
        m_vcpu->interrupt(61);
    }

  private:
    Network() {
        this->m_header.m_magic = 0x74726976;
        this->m_header.m_version = 1;
        this->m_header.m_id = 1;
        this->m_header.m_vendor = 0x554d4551;

        // this->m_header.m_host_features = (1 << 5);
        this->m_header.m_queue_number_max = NumberOfDescriptors;

        m_vcpu = VirtualCPU::current();
        NetworkAdapter<Device>::instance()->attach(this);

        // setup_mac_address();
    }

    // void setup_mac_address() {
    //     // Exemplo: 52:54:00:12:34:56
    //     m_config.mac[0] = 0x52;
    //     m_config.mac[1] = 0x54;
    //     m_config.mac[2] = 0x00;
    //     m_config.mac[3] = 0x12;
    //     m_config.mac[4] = 0x34;
    //     m_config.mac[5] = 0x56;
    // }

  public:
    static constexpr uintptr_t NumberOfDescriptors = 32;
    static constexpr uintptr_t Address = Base;
    static constexpr size_t Size = sizeof(LegacyHeader);

    // struct Config {
    //     uint8_t mac[6];
    //     uint16_t status;
    // } m_config;

  private:
    static constexpr uintptr_t k_rx_queue = 0;
    static constexpr uintptr_t k_tx_queue = 1;
    static inline Network *s_instance = nullptr;
    VirtualCPU *m_vcpu;
};

} // namespace virtio
