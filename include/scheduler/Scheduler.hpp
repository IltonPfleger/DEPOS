#pragma once

#include <Traits.hpp>
#include <architecture/CPU.hpp>
#include <scheduler/FixedCore.hpp>
#include <scheduler/RR.hpp>
#include <utility/Debug.hpp>
#include <utility/collections/Node.hpp>

namespace DEPOS {

class Scheduler {
  public:
    using Criterion = typename Traits<Scheduler>::Criterion;
    using Node      = collections::Node<Thread *, Criterion>;

    constexpr Scheduler()
        : _heads({nullptr}),
          _collection() {}

    Node *remove(int threshold = Criterion::IDLE) {
        int i = Criterion::HIGHER - 1;

        while (i >= threshold) {
            if (Node *next = _collection.remove(i)) {
                head() = next->value();
                return next;
            }
            i--;
        }

        assert(threshold != Criterion::IDLE);
        return nullptr;
    }

    void insert(Node *node) {
        assert(node);
        _collection.insert(node->criterion(), node);
    }

    Thread *current() { return head(); }

  private:
    Thread *&head() { return _heads[CPU::id()]; }

  private:
    Thread *_heads[Traits<CPU>::Active];
    typename Criterion::template Collection<Node> _collection;
};

} // namespace DEPOS
