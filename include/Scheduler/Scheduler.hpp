#pragma once
#include <Scheduler/Queue.hpp>
#define N 5

struct RR {
    static constexpr bool timed = true;
};

template <typename T>
struct Scheduler {
    typedef FIFO<T*> Queue;
    Queue elements[N];
    int i{0};

    T* chose() {
        i--;
        for (int i = N - 1; i >= 0; --i) {
            T* element = elements[i].get();
            if (element) return element;
        }
        return nullptr;
    }

    void remove(T* element) {
        i--;
        elements[element->priority].remove(element);
    }

    void put(T* element) {
        i++;
        elements[element->priority].put(element);
    }

    bool empty() {
        if (i <= 0) return true;
        return false;
    }
};
