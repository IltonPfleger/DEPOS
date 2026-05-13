#pragma once
#include <utility/collections/SimpleList.hpp>

namespace DEPOS::allocators {

template <size_t Max> class Buddy {
    using Node = collections::Node<void>;

    static size_t level(size_t size) {
        size_t l = 0;
        while ((1ULL << l) < size)
            l++;
        return l;
    }

  public:
    Buddy()
        : free_() {}

    [[nodiscard]]
    void *remove(size_t size) {
        Node *node = nullptr;
        size_t n   = level(size);
        size_t i   = n;

        for (; i <= Max; ++i) {
            node = free_[i].remove();
            if (node) break;
        }

        if (!node) return nullptr;

        while (i > n) {
            i--;
            uintptr_t buddy = reinterpret_cast<uintptr_t>(node) ^ (1U << i);
            free_[i].insert(reinterpret_cast<Node *>(buddy));
        }

        return node;
    }

    void insert(void *p, size_t size) {
        uintptr_t address = reinterpret_cast<uintptr_t>(p);
        size_t n          = level(size);

        while (n < Max) {
            uintptr_t buddy = address ^ (1U << n);
            Node *node      = reinterpret_cast<Node *>(buddy);

            if (!free_[n].remove(node)) break;

            if (buddy < address) address = buddy;
            ++n;
        }

        free_[n].insert(reinterpret_cast<Node *>(address));
    }

  private:
    collections::SimpleList<Node> free_[Max + 1];
};

} // namespace DEPOS::allocators
