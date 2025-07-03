#pragma once
#include <Scheduler/Queue.hpp>

template <typename T, int N>
struct Scheduler {
    typedef Stack<T*> Queue;
    Queue elements[N + 1];

    T* chose() {
        for (int i = N; i >= 0; --i) {
            T* element = elements[i].get();
            if (element) return element;
        }
        return nullptr;
    }

    void remove(T* element) { elements[element->priority].remove(element); }

    void put(T* element) { elements[element->priority].put(element); }
};
