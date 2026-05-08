#pragma once

#include <Traits.hpp>
#include <architecture/CPU.hpp>
#include <scheduler/FixedCore.hpp>
#include <scheduler/RR.hpp>
#include <utility/Debug.hpp>
#include <utils/Lists.hpp>

namespace DEPOS {

template <typename T> class Scheduler {
  public:
    using Criterion = typename Traits<T>::Criterion;
    using Link      = Node<T *, Criterion>;

    Scheduler() = default;

    Link *remove(Criterion minimum = Criterion::IDLE) {
        auto i = Criterion::HIGHER - 1;

        while (i >= minimum) {
            if (Link *next = _collection.remove(i)) {
                head() = next->value();
                return next;
            }
            i--;
        }

        ERROR(minimum == Criterion::IDLE);
        return nullptr;
    }

    void insert(Link *node) {
        ERROR(!node);
        _collection.insert(node->criterion(), node);
    }

    T *current() { return head(); }

  private:
    auto &head() { return _heads[CPU::id()]; }

  private:
    T *_heads[Traits<CPU>::Active];
    typename Criterion::template Collection<Link> _collection;
};

} // namespace DEPOS
