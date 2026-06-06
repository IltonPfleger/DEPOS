#ifndef __DEPOS_POLO__
#define __DEPOS_POLO__

#include <utility/collections/FIFO.hpp>

namespace DEPOS {

namespace collections {

template <typename T> class POLO : public FIFO<T> {

  public:
    void insert(T *node) {
        if (!this->head_ || node->criterion() < this->head_->criterion()) {
            FIFO<T>::insert(node);
        } else {
            T *current = this->head_;
            while (current->next && current->next->criterion() <= node->criterion()) {
                current = current->next;
            }
            node->next    = current->next;
            current->next = node;
        }
    }
};

} // namespace collections

} // namespace DEPOS

#endif
