#pragma once
#include <Scheduler/Lists.hpp>
#define N 5

template <typename T>
struct RR {
    static constexpr bool Timed = true;
};

template <typename T>
struct Scheduler {
    POFO<T*, typename T::Priority> list;
	//LIFO<T*> list;

    T* chose() {
        T* element = list.next();
        if (element) return element;
        return nullptr;
    }

    void remove(T* element) { list.remove(element); }

    void insert(T* element) { list.insert(element); }

    bool empty() { return list.empty(); }
};
