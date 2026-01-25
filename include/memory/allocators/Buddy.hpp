#pragma once
#include <utils/Lists.hpp>

namespace Allocators {
template <size_t Max> class Buddy {
    using List = LIFO<void>;
    using Node = typename List::Node;

    static size_t level(size_t size) {
        size_t level = 0;
        while ((1U << level) < size && level <= Max) {
            ++level;
        }
        return level;
    }

  public:
    bool empty() const {
        for (unsigned int i = 0; i < Max + 1; i++) {
            if (!m_free[i].empty()) return false;
        }
        return true;
    }

    size_t max() const {
        for (size_t i = Max;; --i) {
            if (!m_free[i].empty()) {
                return 1 << i;
            }
            if (i == 0) break;
        }
        return 0;
    }

    void *remove(size_t size) {
        Node *node = nullptr;
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
            m_free[i].insert(reinterpret_cast<Node *>(buddy));
        }
        return node;
    }

    void insert(void *ptr, size_t size) {
        size_t n = level(size);
        uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);

        while (n < Max) {
            uintptr_t buddy = addr ^ (1U << n);

            Node *previous = nullptr;
            Node *node = m_free[n].head();

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
        m_free[n].insert(reinterpret_cast<Node *>(addr));
    };

  private:
    List m_free[Max + 1];
};

} // namespace Allocators
