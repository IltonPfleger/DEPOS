#ifndef __DEPOS_MPSC_ALLOCATOR__
#define __DEPOS_MPSC_ALLOCATOR__

#include <Meta.hpp>
#include <Sempahore.hpp>

namespace DEPOS {

namespace collections {

template <typename T, size_t Capacity> class MPMC {

  public:
    MPSC()
        : p_(1) {}

    void insert(T *node) {
        p_.p();
        node->next() = nullptr;
        if (!head_)
            head_ = tail_ = node;
        else
            tail_ = (tail_->next() = node);
        p_.v();
    }

    T *remove() {
        p_.p();
        T *node = head_;
        if (node) head_ = node->next();
        if (!head_) tail_ = nullptr;
        p_.v();
        return node;
    }

  protected:
    Sempahore p_;
    T *head_;
    T *tail_;
};

} // namespace collections

} // namespace DEPOS

#endif
