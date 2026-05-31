#pragma once

#include <Meta.hpp>

namespace DEPOS {

namespace collections {

template <typename V = void, typename P = void, typename D = void> struct Node {
    using Value    = typename Meta::IF<Meta::IsVoid<V>::Result, Meta::Empty, V>::Result;
    using Priority = typename Meta::IF<Meta::IsVoid<P>::Result, Meta::Empty, P>::Result;
    using Next     = typename Meta::IF<Meta::IsVoid<D>::Result, Node *, D *>::Result;

    auto &next(this auto &self) { return self.m_next; }
    void next(this auto &self, auto next) { self.m_next = next; }

    auto &value(this auto &self) { return self.m_value; }
    void value(this auto &self, Value value) { self.m_value = value; }

    auto criterion(this auto &self) { return self.m_criterion; }

    template <typename T, typename U>
    Node(T &&v, U &&p)
        : m_value(static_cast<V &&>(v)),
          m_criterion(static_cast<P &&>(p)),
          m_next(nullptr) {}

    template <typename T>
    Node(T &&v)
        : m_value(static_cast<V &&>(v)),
          m_criterion(),
          m_next(nullptr) {}

    Node()
        : m_value(),
          m_criterion(),
          m_next(nullptr) {}

  public:
    Value m_value;
    Priority m_criterion;
    Next m_next;
};

} // namespace collections

} // namespace DEPOS
