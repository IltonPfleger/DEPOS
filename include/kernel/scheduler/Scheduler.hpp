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

    Link *remove(Criterion threshold = Criterion::IDLE) {
        uint32_t i = Criterion::Levels - 1;

        while (i >= threshold) {
            if (Link *next = m_collection.remove(Criterion(i))) {
                m_heads[head()] = next->value();
                return next;
            }
            i--;
        }
        return nullptr;
    }

    void insert(Link *node) {
        ERROR(!node);
        m_collection.insert(node->criterion(), node);
    }

    auto head() { return CPU::id(); }

    auto *current() { return m_heads[head()]; }

  private:
    static constexpr uint32_t CPUS = Traits<CPU>::Active;

  private:
    Meta::Array<CPUS, T *> m_heads;
    typename Criterion::template Collection<Link> m_collection;
};

} // namespace DEPOS
