#pragma once

#include <types.hpp>

namespace DEPOS {

namespace virtio {

struct RingDescriptor {
    uint64_t address;
    uint32_t length;
    uint16_t flags;
    uint16_t next;
} __attribute__((packed));

struct RingAvailable {
    uint16_t flags;
    uint16_t index;
    uint16_t *ring() { return reinterpret_cast<uint16_t *>(reinterpret_cast<uintptr_t>(this) + 4); };
} __attribute__((packed));

struct RingUsedElement {
    uint32_t id;
    uint32_t length;
};

struct RingUsed {
    uint16_t flags;
    uint16_t index;
    RingUsedElement *ring() { return reinterpret_cast<RingUsedElement *>(reinterpret_cast<uintptr_t>(this) + 4); };
} __attribute__((packed));

class Queue {
  public:
    uint16_t alloc() { return m_available->ring()[m_last_available_index++ % m_size]; }

    auto *descriptor(uint16_t id) { return &m_descriptors[id]; }

    void free(uint16_t id, uint32_t length = 0) {
        uint16_t index        = m_used->index % m_size;
        m_used->ring()[index] = {.id = id, .length = length};
        m_used->index++;
    }

    bool available() const {
        if (!m_available) return false;
        return m_last_available_index != m_available->index;
    }

    template <typename Callback> void produce(size_t total_size, Callback &&cb) {
        if (!available()) return;

        uint16_t id = alloc();

        auto *desc = descriptor(id);
        auto *dst  = reinterpret_cast<unsigned char *>(desc->address);

        cb(dst);

        desc->length = total_size;
        desc->flags  = 0;

        free(id, total_size);
    }

    template <typename Callback> void consume(int head, Callback &&cb) {
        int current = head;

        while (true) {
            auto *desc = descriptor(current);
            cb(desc);

            if (!(desc->flags & 0x1)) break;

            current = desc->next;
        }
    }

    template <typename Callback> void process(Callback &&cb) {
        if (!available()) return;

        int head     = alloc();
        size_t total = 0;

        consume(head, [&](auto *desc) {
            cb(desc->address, desc->length);
            total += desc->length;
        });

        free(head, total);
    }

    Queue() = default;

    Queue(uintptr_t address, uint32_t size, uint32_t align)
        : m_address(address),
          m_size(size),
          m_last_available_index(0) {

        m_descriptors = reinterpret_cast<RingDescriptor *>(address);
        address += size * sizeof(RingDescriptor);
        m_available = reinterpret_cast<RingAvailable *>(address);
        address += 2 + 2 + (2 * size) + 2;
        address = (address + align - 1) & ~(align - 1);
        m_used  = reinterpret_cast<RingUsed *>(address);
    }

  public:
    uintptr_t m_address             = 0;
    uint32_t m_size                 = 0;
    uint16_t m_last_available_index = 0;
    RingDescriptor *m_descriptors   = nullptr;
    RingAvailable *m_available      = nullptr;
    RingUsed *m_used                = nullptr;
};

} // namespace virtio

} // namespace DEPOS
