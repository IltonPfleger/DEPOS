#pragma once

#include <architecture/CPU.hpp>

namespace DEPOS {

namespace collections {

template <typename T> class MPSC {
  public:
    MPSC() { head_ = tail_; }

    // Multiple-Producers
    void insert(T *node) {
        node->next() = nullptr;
        T *old;
        do {
            old = tail_;
        } while (!CPU::Atomic::cas(tail_, old, node));
        if (old) old->next() = node;
    }

    // Single-Consumer
    T *remove() {
        T *head = head_;
        T *next = head->next();

        if (head == nullptr) {
            if (tail_ == nullptr) return nullptr;
            head_ = head = tail_;
        }

        while (head->next() == nullptr && head != tail_) {
        }

        head_ = head->next();

        return next;
    }

  protected:
    T *head_ = nullptr;
    T *tail_ = nullptr;
};

} // namespace collections

} // namespace DEPOS
