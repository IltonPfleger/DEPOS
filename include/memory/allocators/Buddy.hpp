#pragma once
#include <utils/Lists.hpp>

namespace Allocators {
template <size_t Max> class Buddy {
    using Entry = Node<void>;
    using List = FIFO<Entry>;

    static size_t level(size_t size) {
        if (size <= 1) return 0;
        size_t l = 64 - __builtin_clzll(size - 1);
        return (l > Max) ? Max : l;
    }

  public:
    Buddy() = default;

    void *remove(size_t size) {
        Entry *node = nullptr;
        size_t n = level(size);
        size_t i = n;
        for (; i <= Max; ++i) {
            node = m_free[i].remove();
            if (node) break;
        }
        if (!node) return nullptr;
        while (i > n) {
            i--;
            size_t half = 1 << i;
            uintptr_t buddy = reinterpret_cast<uintptr_t>(node) + half;
            m_free[i].insert(reinterpret_cast<Entry *>(buddy));
        }
        return node;
    }

    void insert(void *ptr, size_t size) {
        size_t n = level(size);
        uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);

        while (n < Max) {
            uintptr_t buddy = addr ^ (1U << n);
            Entry *node = reinterpret_cast<Entry *>(buddy);

            if (m_free[n].remove(node)) {
                if (buddy < addr) addr = buddy;
                ++n;
            } else {
                break;
            }
        }

        m_free[n].insert(reinterpret_cast<Entry *>(addr));
    }

  private:
    List m_free[Max + 1];
};

} // namespace Allocators
