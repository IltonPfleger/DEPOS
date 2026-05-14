#pragma once

#include <utility/collections/SimpleList.hpp>

namespace DEPOS::allocators {

template <size_t Min, size_t Max> class Buddy {

    typedef collections::Node<void> Node;

  public:
    Buddy()
        : free_() {}

    [[nodiscard]]
    void *remove(size_t size) {
        Node *node = nullptr;
        size_t n   = bucket(size);
        size_t i   = n;

        for (; i <= Maximum; ++i) {
            node = free_[i].remove();
            if (node) break;
        }

        if (!node) return nullptr;

        while (i > n) {
            i--;
            uintptr_t b = buddy(reinterpret_cast<uintptr_t>(node), i);
            free_[i].insert(reinterpret_cast<Node *>(b));
        }

        return node;
    }

    void insert(void *p, size_t size) {
        uintptr_t address = reinterpret_cast<uintptr_t>(p);
        size_t n          = bucket(size);

        while (n < Maximum) {
            uintptr_t b = buddy(address, n);
            Node *node  = reinterpret_cast<Node *>(b);

            if (!free_[n].remove(node)) break;

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
        return address ^ size;
    }

  private:
    static constexpr size_t Minimum = (Min >= sizeof(Node)) ? log2ceil(Min) : log2ceil(sizeof(Node));
    static constexpr size_t Maximum = log2ceil(Max);

  private:
    collections::SimpleList<Node> free_[Maximum - Minimum + 1];
};

} // namespace DEPOS::allocators
