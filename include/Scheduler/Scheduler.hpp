#pragma once
#include <Scheduler/Lists.hpp>

template <typename T>
struct RR {
    static constexpr bool Timed = false;
};

template <typename T>
struct RateMonotonic {
    static constexpr bool Timed = true;
};

template <typename T>
struct Scheduler {
    POFO<T *> list;

    T *chose() {
        T *element = list.next();
        return element;
    }

    void remove(T *element) { list.remove(element); }

    void insert(T *element) { list.insert(element); }

    bool empty() { return list.empty(); }
};
