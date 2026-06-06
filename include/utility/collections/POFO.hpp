#ifndef __DEPOS_POFO__
#define __DEPOS_POFO__

#include <utility/collections/FIFO.hpp>

namespace DEPOS::collections {

template <typename T, bool Atomic = false> class POFO : public FIFO<T, Atomic> {
  public:
    void insert(T *node) {
        bool enabled = FIFO<T, Atomic>::lock();
        if (!this->head_ || node->criterion() < this->head_->criterion()) {
            node->next  = this->head_;
            this->head_ = node;
            if (!this->tail_) this->tail_ = node;
        } else {
            T *current = this->head_;
            while (current->next && current->next->criterion() <= node->criterion()) {
                current = current->next;
            }
            node->next    = current->next;
            current->next = node;
        }
        FIFO<T, Atomic>::unlock(enabled);
    }
};

} // namespace DEPOS::collections

#endif
