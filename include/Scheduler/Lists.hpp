#pragma once

#include <Memory.hpp>
#include <Meta.hpp>

#pragma once

#include <Memory.hpp>
#include <Meta.hpp>

template <typename T>
struct Base {
    struct Element {
        T value;
        Element *next;
    };

    bool empty() const { return head == nullptr; }

    T next() {
        if (empty()) return T{};
        Element *node = head;
        head          = node->next;
        T value       = node->value;
        if (empty()) tail = nullptr;
        delete node;
        return value;
    }

    void remove(T value) {
        if (empty()) return;

        Element *current  = head;
        Element *previous = nullptr;

        while (current && current->value != value) {
            previous = current;
            current  = current->next;
        }

        if (!current) return;

        if (current == head) {
            head = current->next;
            if (empty()) tail = nullptr;
        } else {
            previous->next = current->next;
            if (current == tail) {
                tail = previous;
            }
        }
        delete current;
    }

    Element *head = nullptr;
    Element *tail = nullptr;
};

template <typename T>
struct LIFO : Base<T> {
    using Base<T>::empty;
    using Base<T>::head;
    using Base<T>::remove;
    using typename Base<T>::Element;

    void insert(T value) { head = new (Memory::SYSTEM) Element{value, head}; }
};

template <typename T>
struct FIFO : Base<T> {
    using Base<T>::empty;
    using Base<T>::head;
    using Base<T>::remove;
    using Base<T>::tail;
    using typename Base<T>::Element;

    void insert(T value) {
        Element *node = new (Memory::SYSTEM) Element{value, nullptr};

        if (empty()) {
            head = tail = node;
            return;
        }

        tail->next = node;
        tail       = node;
    }
};

template <typename T, typename R>
struct POFO : Base<T> {
    using Base<T>::empty;
    using Base<T>::head;
    using Base<T>::remove;
    using typename Base<T>::Element;

    void insert(T value) {
        Element *node = new (Memory::SYSTEM) Element{value, nullptr};

        if (empty() || priority(value) > priority(head->value)) {
            node->next = head;
            head       = node;
            return;
        }

        Element *current = head;
        while (current->next && priority(value) < priority(current->next->value)) {
            current = current->next;
        }

        node->next    = current->next;
        current->next = node;
    }

    constexpr R priority(T &value) {
        if constexpr (Meta::IS_POINTER<T>::Result) {
            return value->priority;
        } else {
            return value.priority;
        }
    }
};

// template <typename Derived, typename T>
// struct Base {
//	struct Element {
//		T value;
//		Element *next;
//	};
//
//	bool empty() const { return head == nullptr; }
//
//	T next() {
//		if (empty()) return T{};
//		Element *node = head;
//		head          = node->next;
//		T value       = node->value;
//		delete node;
//		if (empty()) tail = nullptr;
//		return value;
//	}
//
//	void remove(T value) {
//		if (empty()) return;
//
//		Element *current  = head;
//		Element *previous = nullptr;
//
//		while (current && current->value != value) {
//			previous = current;
//			current  = current->next;
//		}
//
//		if (!current) return;
//
//		if (current == head) {
//			head = current->next;
//			if (empty()) tail = nullptr;
//		} else {
//			previous->next = current->next;
//			if (current == tail) {
//				tail = previous;
//			}
//		}
//		delete current;
//	}
//
//	Element *head = nullptr;
//	Element *tail = nullptr;
// };
//
// template <typename T>
// struct LIFO : Base<LIFO<T>, T> {
//	using Parent = Base<LIFO, T>;
//	using Parent::empty;
//	using Parent::head;
//	using Parent::remove;
//	using typename Parent::Element;
//	void insert(T value) { head = new (Memory::SYSTEM) Element{value, head}; }
// };
//
// template <typename T>
// struct FIFO : Base<FIFO<T>, T> {
//	using Parent = Base<FIFO<T>, T>;
//	using Parent::empty;
//	using Parent::head;
//	using Parent::remove;
//	using Parent::tail;
//	using typename Parent::Element;
//
//	void insert(T value) {
//		Element *node = new (Memory::SYSTEM) Element{value, nullptr};
//
//		if (empty()) {
//			head = tail = node;
//			return;
//		}
//
//		tail->next = node;
//		tail       = node;
//	}
// };
//
// template <typename T, typename R>
// struct POFO : Base<POFO<T, R>, T> {
//	using Parent = Base<POFO<T, R>, T>;
//	using Parent::empty;
//	using Parent::head;
//	using Parent::remove;
//	using typename Parent::Element;
//
//	void insert(T value) {
//		Element *node = new (Memory::SYSTEM) Element{value, nullptr};
//
//		if (empty() || priority(value) > priority(head->value)) {
//			node->next = head;
//			head       = node;
//			return;
//		}
//
//		Element *current = head;
//		while (current->next && priority(value) < priority(current->next->value)) {
//			current = current->next;
//		}
//
//		node->next    = current->next;
//		current->next = node;
//	}
//
//	constexpr R priority(T &value) {
//		if constexpr (Meta::IS_POINTER<T>::Result) {
//			return value->priority;
//		} else {
//			return value.priority;
//		}
//	}
// };
