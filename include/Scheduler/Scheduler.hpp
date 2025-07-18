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
    using Queue = POFO<T*>;
    Queue list;

    T *chose() {
        auto *element = list.next();
        return element->value;
    }

    void remove(T *element) { list.remove(element->link); }

    void insert(T *element) { list.insert(element->link); }

    bool empty() { return list.empty(); }
};
