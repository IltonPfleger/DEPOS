#pragma once

#include <types.hpp>
#include <utility/collections/Node.hpp>

namespace DEPOS {

namespace collections {

template <typename T> class SimpleList {
  public:
    SimpleList()
        : _head(nullptr) {}

    T *head(this auto &&self) { return self._head; }

    void insert(T *node) {
        if (!node) return;

        node->next(this->_head);
        this->_head = node;
    }

    T *remove() {
        if (!this->_head) return nullptr;

        T *node     = this->_head;
        this->_head = node->next();
        node->next(nullptr);

        return node;
    }

    bool remove(T *target) {
        if (!this->_head || !target) return false;

        T *previous = nullptr;
        T *current  = this->_head;

        while (current && current != target) {
            previous = current;
            current  = current->next();
        }

        if (!current) return false;

        if (previous)
            previous->next(current->next());
        else
            this->_head = current->next();

        current->next(nullptr);

        return true;
    }

  private:
    T *_head;
};

} // namespace collections

} // namespace DEPOS
