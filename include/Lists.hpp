#pragma once

#include <Types.hpp>

template <typename T>
struct Node {
    const T& data;
    Node* next;
};

template <>
struct Node<void> {
    Node* next;
};

template <typename T>
class LinkedListBase {
   public:
    using DataType = T;
    using NodeType = Node<T>;
    bool empty() const { return mhead == nullptr; }
    const NodeType* head() const { return mhead; }

   protected:
    NodeType* mhead = nullptr;
};

template <typename T>
class LIFO : public LinkedListBase<T> {
   public:
    using Base     = LinkedListBase<T>;
    using NodeType = typename Base::NodeType;

    void insert(NodeType* node) {
        node->next  = this->mhead;
        this->mhead = node;
    }

    NodeType* remove() {
        if (!this->mhead) return nullptr;
        NodeType* node = this->mhead;
        this->mhead    = node->next;
        return node;
    }
};

template <uintptr_t BASE, size_t MAX>
class BuddyAllocator {
    using Base     = LIFO<void>;
    using NodeType = typename Base::NodeType;

    static size_t level(size_t size) {
        size_t level = 0;
        while ((1U << level) < size && level <= MAX) {
            ++level;
        }
        return level;
    }

   public:
    void* remove(size_t size) {
        size_t n = level(size);
        for (size_t i = n; i <= MAX; ++i) {
            if (mfree[i].empty()) continue;
            NodeType* node = mfree[i].remove();
            while (i > n) {
                i--;
                size_t half     = 1 << i;
                uintptr_t buddy = reinterpret_cast<uintptr_t>(node) + half;
                mfree[i].insert(reinterpret_cast<NodeType*>(buddy));
            }
            return reinterpret_cast<void*>(node);
        }
        return nullptr;
    }

    void insert(void* ptr, size_t size) {
        size_t n       = level(size);
        uintptr_t addr = reinterpret_cast<uintptr_t>(ptr);

        while (n < MAX) {
            size_t bsize    = (1 << n);
            size_t offset   = addr - BASE;
            uintptr_t buddy = BASE + (offset ^ bsize);

            NodeType* previous = nullptr;
            NodeType* node     = const_cast<NodeType*>(mfree[n].head());

            while (node) {
                if (reinterpret_cast<uintptr_t>(node) == buddy) break;
                previous = node;
                node     = node->next;
            }

            if (!node) break;

            if (previous) {
                previous->next = node->next;
            } else {
                mfree[n].remove();
            }

            if (buddy < addr) addr = buddy;
            ++n;
        }
        mfree[n].insert(reinterpret_cast<NodeType*>(addr));
    };

   private:
    Base mfree[MAX + 1];
};

/* ----------------------------------- O L D -----------------------------------*/

template <typename T>
struct Element {
    T value;
    unsigned long long rank;
    Element* next = nullptr;
};

template <typename T>
struct LinkedList {
    using Node = Element<T>;

    Node* mhead = nullptr;
    Node* _tail = nullptr;

    bool empty() const { return mhead == nullptr; }

    void push_front(Node* e) {
        e->next = mhead;
        mhead   = e;
        if (!_tail) _tail = e;
    }

    void push_back(Node* e) {
        e->next = nullptr;
        if (!mhead) {
            mhead = _tail = e;
        } else {
            _tail->next = e;
            _tail       = e;
        }
    }

    void push_sorted(Node* e) {
        e->next = nullptr;
        if (!mhead || e->rank < mhead->rank) {
            push_front(e);
        } else {
            Node* current = mhead;
            while (current->next && e->rank >= current->next->rank) {
                current = current->next;
            }
            e->next       = current->next;
            current->next = e;
            if (!e->next) _tail = e;
        }
    }

    Node* remove_front() {
        if (!mhead) return nullptr;
        Node* e = mhead;
        mhead   = e->next;
        if (!mhead) _tail = nullptr;
        e->next = nullptr;
        return e;
    }

    Node* remove_back() {
        if (!mhead) return nullptr;
        if (mhead == _tail) {
            Node* e = mhead;
            mhead = _tail = nullptr;
            return e;
        }
        Node* current = mhead;
        while (current->next != _tail) {
            current = current->next;
        }
        Node* e     = _tail;
        _tail       = current;
        _tail->next = nullptr;
        return e;
    }

    void remove(Node* e) {
        if (!mhead) return;
        Node* current  = mhead;
        Node* previous = nullptr;
        while (current && current != e) {
            previous = current;
            current  = current->next;
        }
        if (!current) return;
        if (current == mhead) {
            mhead = current->next;
        } else {
            previous->next = current->next;
        }
        if (current == _tail) {
            _tail = previous;
        }
        e->next = nullptr;
        if (!mhead) _tail = nullptr;
    }
};

template <typename T>
struct FIFO : private LinkedList<T> {
    using LinkedList<T>::empty;
    using LinkedList<T>::remove;
    using Node = typename LinkedList<T>::Node;

    void insert(Node* value) { LinkedList<T>::push_back(value); }
    Node* next() { return LinkedList<T>::remove_front(); }
};

template <typename T>
struct POFO : private LinkedList<T> {
    using LinkedList<T>::empty;
    using LinkedList<T>::remove;
    using Node = typename LinkedList<T>::Node;

    void insert(Node* value) { LinkedList<T>::push_sorted(value); }
    Node* next() { return LinkedList<T>::remove_front(); }
};
