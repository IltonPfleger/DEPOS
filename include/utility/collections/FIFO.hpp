#ifndef __DEPOS_FIFO__
#define __DEPOS_FIFO__

namespace DEPOS {

namespace collections {

template <typename T> class FIFO {
  public:
    FIFO() = default;

    T *head() { return head_; }

    void insert(T *node) {
        node->next() = nullptr;
        if (!head_)
            head_ = tail_ = node;
        else
            tail_ = (tail_->next() = node);
    }

    T *remove() {
        T *node = head_;
        if (!node) return nullptr;
        head_ = node->next();
        if (!head_) tail_ = nullptr;
        return node;
    }

  protected:
    T *head_;
    T *tail_;
};

} // namespace collections

} // namespace DEPOS

#endif
