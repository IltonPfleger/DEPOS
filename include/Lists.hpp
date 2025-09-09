#pragma once

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
    bool empty() const { return _head == nullptr; }

   protected:
    NodeType* _head = nullptr;
};

template <typename T>
class LIFO : LinkedListBase<T> {
    using Base = LinkedListBase<T>;

   public:
    using DataType = Base::DataType;
    using NodeType = Base::NodeType;

    void insert(NodeType* node) {
        node->next  = this->_head;
        this->_head = node;
    };

    NodeType* remove() {
        NodeType* node = this->_head;
        this->_head    = this->_head->next;
        return node;
    }
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

    Node* _head = nullptr;
    Node* _tail = nullptr;

    bool empty() const { return _head == nullptr; }

    void push_front(Node* e) {
        e->next = _head;
        _head   = e;
        if (!_tail) _tail = e;
    }

    void push_back(Node* e) {
        e->next = nullptr;
        if (!_head) {
            _head = _tail = e;
        } else {
            _tail->next = e;
            _tail       = e;
        }
    }

    void push_sorted(Node* e) {
        e->next = nullptr;
        if (!_head || e->rank < _head->rank) {
            push_front(e);
        } else {
            Node* current = _head;
            while (current->next && e->rank >= current->next->rank) {
                current = current->next;
            }
            e->next       = current->next;
            current->next = e;
            if (!e->next) _tail = e;
        }
    }

    Node* remove_front() {
        if (!_head) return nullptr;
        Node* e = _head;
        _head   = e->next;
        if (!_head) _tail = nullptr;
        e->next = nullptr;
        return e;
    }

    Node* remove_back() {
        if (!_head) return nullptr;
        if (_head == _tail) {
            Node* e = _head;
            _head = _tail = nullptr;
            return e;
        }
        Node* current = _head;
        while (current->next != _tail) {
            current = current->next;
        }
        Node* e     = _tail;
        _tail       = current;
        _tail->next = nullptr;
        return e;
    }

    void remove(Node* e) {
        if (!_head) return;
        Node* current  = _head;
        Node* previous = nullptr;
        while (current && current != e) {
            previous = current;
            current  = current->next;
        }
        if (!current) return;
        if (current == _head) {
            _head = current->next;
        } else {
            previous->next = current->next;
        }
        if (current == _tail) {
            _tail = previous;
        }
        e->next = nullptr;
        if (!_head) _tail = nullptr;
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
