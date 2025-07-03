#pragma once
#include <Scheduler/Queue.hpp>

template <typename T>
struct Scheduler {
    typedef Stack<T*> Queue;
    Queue elements[5];

    T* chose() {
        for (int i = 4; i >= 0; --i) {
            T* element = elements[i].get();
            if (element) return element;
        }
        return nullptr;
    }

    void remove(T* element) { elements[element->priority].remove(element); }

    void put(T* element) { elements[element->priority].put(element); }
};
