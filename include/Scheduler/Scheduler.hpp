#pragma once
#include <Scheduler/Lists.hpp>
#define N 5

template <typename T>
struct RR {
    static constexpr bool Timed = true;
    using List                  = FIFO<T*>;
};

template <typename T>
struct Scheduler {
    using Criterion = Traits<Scheduler<T>>::Criterion;
    using List      = Criterion::List;
    List lists[N];

    T* chose() {
        for (int j = N - 1; j >= 0; --j) {
            T* element = lists[j].get();
            if (element) return element;
        }
        return nullptr;
    }

    void remove(T* element) { lists[element->priority].remove(element); }

    void put(T* element) { lists[element->priority].put(element); }

    bool empty() {
        for (int j = 0; j < N; ++j) {
            if (!lists[j].empty()) {
                return false;
            }
        }
        return true;
    }
};
