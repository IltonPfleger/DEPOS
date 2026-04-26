#pragma once

#include <types.hpp>
#include <utils/collections/FIFO.hpp>
#include <utils/collections/LIFO.hpp>
#include <utils/collections/Node.hpp>

namespace DEPOS::allocators {

template <size_t Max> class Buddy {
    using Node     = collections::Node<void>;
    using FreeList = collections::LIFO<Node>;

    static size_t level(size_t size) {
        if (size <= 1) {
            return 0;
        }

        size_t l = 64 - __builtin_clzll(size - 1);
        return (l > Max) ? Max : l;
    }

  public:
    Buddy() = default;

    void *remove(size_t size) {
        const size_t target = level(size);

        Node *node = nullptr;
        size_t i   = target;

        for (; i <= Max; ++i) {
            node = m_free[i].remove();
            if (node) {
                break;
            }
        }

        if (!node) {
            return nullptr;
        }

        while (i > target) {
            --i;

            const size_t blockSize = size_t(1) << i;
            uintptr_t buddyAddr    = reinterpret_cast<uintptr_t>(node) + blockSize;

            m_free[i].insert(reinterpret_cast<Node *>(buddyAddr));
        }

        return node;
    }

    void insert(void *ptr, size_t size) {
        size_t index   = level(size);
        uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);

        while (index < Max) {
            uintptr_t buddyAddr = addr ^ (uintptr_t(1) << index);
            Node *buddyNode     = reinterpret_cast<Node *>(buddyAddr);

            if (m_free[index].remove(buddyNode)) {
                if (buddyAddr < addr) {
                    addr = buddyAddr;
                }
                ++index;
            } else {
                break;
            }
        }

        m_free[index].insert(reinterpret_cast<Node *>(addr));
    }

  private:
    FreeList m_free[Max + 1];
};

} // namespace DEPOS::allocators
