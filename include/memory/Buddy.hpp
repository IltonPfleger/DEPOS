#pragma once
#include <utils/Lists.hpp>

template <size_t MAX>
class BuddyAllocator {
    using List = LIFO<void>;
    using Node = typename List::Node;

    static size_t level(size_t size) {
        size_t level = 0;
        while ((1U << level) < size && level <= MAX) {
            ++level;
        }
        return level;
    }

   public:
    void* remove(size_t size) {
        Node* node = nullptr;
        size_t n   = level(size);
        size_t i   = n;
        for (; i <= MAX; ++i) {
            node = m_free[i].remove();
            if (node) break;
        }
        while (i > n) {
            i--;
            size_t half     = 1 << i;
            uintptr_t buddy = reinterpret_cast<uintptr_t>(node) + half;
            m_free[i].insert(reinterpret_cast<Node*>(buddy));
        }
        return node;
    }

    void insert(void* ptr, size_t size) {
        size_t n       = level(size);
        uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);

        while (n < MAX) {
            uintptr_t buddy = addr ^ (1U << n);

            Node* previous = nullptr;
            Node* node     = m_free[n].head();

            while (node) {
                if (reinterpret_cast<uintptr_t>(node) == buddy) break;
                previous = node;
                node     = node->next;
            }

            if (!node) break;

            if (previous)
                previous->next = node->next;
            else
                m_free[n].remove();

            if (buddy < addr) addr = buddy;
            ++n;
        }
        m_free[n].insert(reinterpret_cast<Node*>(addr));
    };

   private:
    List m_free[MAX + 1];
};
