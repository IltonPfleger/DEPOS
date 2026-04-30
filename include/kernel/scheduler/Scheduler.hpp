#pragma once
#include <Traits.hpp>
#include <architecture/CPU.hpp>
#include <kernel/scheduler/FixedCore.hpp>
#include <kernel/scheduler/RR.hpp>
#include <shared/console/Debug.hpp>
#include <utils/collections/Node.hpp>

namespace DEPOS {

template <typename T> class Scheduler {
  public:
    using Criterion = typename Traits<T>::Criterion;
    using Link      = collections::Node<T *, Criterion>;

    Scheduler() = default;

    Link *remove(Criterion minimum = Criterion::IDLE) {
        auto i = Criterion::HIGHER - 1;

        while (i >= minimum) {
            if (Link *next = m_collection.remove(i)) {
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
        m_collection.insert(node->criterion(), node);
    }

    T *current() { return head(); }

  private:
    auto &head() { return m_heads[CPU::id()]; }

  private:
    meta::Array<Traits<CPU>::Active, T *> m_heads;
    typename Criterion::template Collection<Link> m_collection;
};

} // namespace DEPOS
