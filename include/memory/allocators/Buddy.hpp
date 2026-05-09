#pragma once
#include <utility/collections/SimpleList.hpp>

namespace DEPOS {

namespace allocators {

template <size_t Max> class Buddy {
    using Node = collections::Node<void>;

    static size_t level(size_t size) {
        if (size <= 1) return 0;
        size_t l = 64 - __builtin_clzll(size - 1);
        return (l > Max) ? Max : l;
    }

  public:
    Buddy() = default;

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
            size_t half     = 1 << i;
            uintptr_t buddy = reinterpret_cast<uintptr_t>(node) + half;
            free_[i].insert(reinterpret_cast<Node *>(buddy));
        }
        return node;
    }

    void insert(void *ptr, size_t size) {
        size_t n       = level(size);
        uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);

        while (n < Max) {
            uintptr_t buddy = addr ^ (1U << n);
            Node *node      = reinterpret_cast<Node *>(buddy);

            if (free_[n].remove(node)) {
                if (buddy < addr) addr = buddy;
                ++n;
            } else {
                break;
            }
        }

        free_[n].insert(reinterpret_cast<Node *>(addr));
    }

  private:
    collections::SimpleList<Node> free_[Max + 1];
};

} // namespace allocators

} // namespace DEPOS
