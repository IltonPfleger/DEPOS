#pragma once

#include <cstddef>
#include <cstdint>
#include <utility/collections/SimpleList.hpp>

namespace DEPOS::allocators {

template <size_t Minimum, size_t Maximum> class Buddy {
    static_assert(Minimum <= Maximum);

    typedef collections::Node<void> Node;

    static constexpr size_t MaxIndex = Maximum - Minimum;

  public:
    constexpr Buddy()
        : free_() {}

    [[nodiscard]]
    void *remove(size_t size) {
        // FIX 1: Guard against requesting sizes larger than the maximum block
        // Without this, bucket(size) returns an out-of-bounds index.
        if (size == 0 || size > (1ULL << Maximum)) return nullptr;

        Node *node = nullptr;
        size_t n   = bucket(size);
        size_t i   = n;

        // FIX 2: Upper bound is MaxIndex (Maximum - Minimum), not Maximum.
        // Array size is (Maximum - Minimum + 1).
        for (; i <= MaxIndex; ++i) {
            node = free_[i].remove();
            if (node) break;
        }

        if (!node) return nullptr;

        // Split blocks down to the requested size
        while (i > n) {
            i--;
            uintptr_t b = buddy(reinterpret_cast<uintptr_t>(node), i);
            free_[i].insert(reinterpret_cast<Node *>(b));
        }

        return node;
    }

    void insert(void *p, size_t size) {
        // Guard against invalid frees
        if (!p || size == 0 || size > (1ULL << Maximum)) return;

        uintptr_t address = reinterpret_cast<uintptr_t>(p);
        size_t n          = bucket(size);

        // FIX 3: Upper bound is MaxIndex, not Maximum.
        while (n < MaxIndex) {
            uintptr_t b      = buddy(address, n);
            Node *buddy_node = reinterpret_cast<Node *>(b);

            // Attempt to remove the buddy from the free list to merge them
            if (!free_[n].remove(buddy_node)) break;

            // Merging: the new block's address is the lower of the two buddies
            if (b < address) address = b;
            ++n;
        }

        free_[n].insert(reinterpret_cast<Node *>(address));
    }

  private:
    constexpr static size_t log2ceil(size_t size) {
        size_t l = 0;
        while ((1ULL << l) < size)
            l++;
        return l;
    }

    static constexpr size_t bucket(size_t size) {
        size_t b = log2ceil(size);
        if (b < Minimum) b = Minimum;
        return b - Minimum;
    }

    static constexpr uintptr_t buddy(uintptr_t address, size_t bucket) {
        uintptr_t size = 1ULL << (bucket + Minimum);

        // NOTE: The XOR trick requires the base address of your managed memory
        // to be properly aligned (a multiple of `1ULL << Maximum`).
        return address ^ size;
    }

  private:
    collections::SimpleList<Node> free_[MaxIndex + 1];
};

} // namespace DEPOS::allocators
