#pragma once

#include <types.hpp>
#include <utility/Debug.hpp>

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
    Queue() = default;

    Queue(uintptr_t address, uint32_t size, uint32_t align)
        : m_address(address),
          m_size(size),
          m_last_available_index(0) {

        m_descriptors = reinterpret_cast<RingDescriptor *>(address);

        address += sizeof(RingDescriptor) * size;

        m_available = reinterpret_cast<RingAvailable *>(address);

        address += sizeof(uint16_t);
        address += sizeof(uint16_t);
        address += sizeof(uint16_t) * size;
        address += sizeof(uint16_t);

        address = (address + align - 1) & ~(align - 1);

        m_used = reinterpret_cast<RingUsed *>(address);
    }

    bool available() { return m_last_available_index != m_available->index; }

    int alloc() { return m_available->ring()[m_last_available_index++ % m_size]; }

    RingDescriptor *get(uint32_t id) {
        assert(id <= m_size);
        return &m_descriptors[id];
    }

    void free(unsigned int id, unsigned int length = 0) {
        uint16_t index               = m_used->index % m_size;
        m_used->ring()[index].id     = id;
        m_used->ring()[index].length = length;
        m_used->index++;
    }

    static constexpr uintptr_t size(uint32_t size, uint32_t align) {
        uintptr_t address = 0;

        address += size * sizeof(RingDescriptor);

        address += 2 + 2 + (2 * size) + 2;

        address = (address + align - 1) & ~(align - 1);

        address += sizeof(uint16_t);
        address += sizeof(uint16_t);
        address += sizeof(RingUsedElement) * size;

        return address;
    }

  public:
    uintptr_t m_address;
    uint32_t m_size;
    uint16_t m_last_available_index;
    RingDescriptor *m_descriptors;
    RingAvailable *m_available;
    RingUsed *m_used;
};

} // namespace virtio

} // namespace DEPOS
