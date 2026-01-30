#pragma once
#include <utils/Lists.hpp>

namespace Allocators {
template <size_t Max> class Buddy {
    using Entry = Node<void>;
    using List = LIFO<Entry>;

    static size_t level(size_t size) {
        size_t level = 0;
        while ((1U << level) < size && level <= Max) {
            ++level;
        }
        return level;
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

            Entry *previous = nullptr;
            Entry *node = m_free[n].head();

            while (node) {
                if (reinterpret_cast<uintptr_t>(node) == buddy) break;
                previous = node;
                node = node->next;
            }

            if (!node) break;

            if (previous)
                previous->next = node->next;
            else
                m_free[n].remove();

            if (buddy < addr) addr = buddy;
            ++n;
        }
        m_free[n].insert(reinterpret_cast<Entry *>(addr));
    };

  private:
    List m_free[Max + 1];
};

} // namespace Allocators
