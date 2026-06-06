#ifndef __DEPOS_FIFO__
#define __DEPOS_FIFO__

#include <Spin.hpp>
#include <architecture/CPU.hpp>

namespace DEPOS::collections {

template <typename T, bool Atomic = false> class FIFO {
  public:
    constexpr FIFO()
        : head_(nullptr),
          tail_(nullptr) {}

    T *head() { return head_; }
    T *tail() { return tail_; }

    void insert(T *node) {
        bool enabled = lock();
        node->next   = nullptr;
        if (!head_)
            head_ = tail_ = node;
        else
            tail_ = (tail_->next = node);
        unlock(enabled);
    }

    T *remove() {
        bool enabled = lock();
        T *node      = head_;
        if (node) head_ = node->next;
        if (!head_) tail_ = nullptr;
        unlock(enabled);
        return node;
    }

  protected:
    [[nodiscard]]
    bool lock() {
        if constexpr (Atomic) {
            bool enabled = CPU::Interrupt::disable();
            lock_.acquire();
            return enabled;
        } else {
            return false;
        }
    }

    void unlock(bool enabled) {
        if constexpr (Atomic) {
            lock_.release();
            if (enabled) CPU::Interrupt::enable();
        }
    }

  protected:
    T *head_;
    T *tail_;
    Meta::IF<Atomic, Spin, Meta::Empty>::Result lock_;
};

} // namespace DEPOS::collections

#endif
