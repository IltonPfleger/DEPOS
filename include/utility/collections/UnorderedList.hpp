#pragma once

#include <types.hpp>
#include <utility/collections/Node.hpp>

namespace DEPOS::collections {

template <typename T> class UnorderedList {
  public:
    constexpr UnorderedList()
        : head_(nullptr) {}

    T *head() { return head_; }

    void insert(T *node) {
        if (!node) return;
        node->next  = this->head_;
        this->head_ = node;
    }

    T *remove() {
        if (!this->head_) return nullptr;
        T *node     = this->head_;
        this->head_ = node->next;
        node->next  = nullptr;
        return node;
    }

    bool remove(T *target) {
        if (!this->head_ || !target) return false;

        T *previous = nullptr;
        T *current  = this->head_;

        while (current && current != target) {
            previous = current;
            current  = current->next;
        }

        if (!current) return false;

        if (previous) {
            previous->next = current->next;
        } else {
            this->head_ = current->next;
        }

        current->next = nullptr;

        return true;
    }

  private:
    T *head_;
};

} // namespace DEPOS::collections
