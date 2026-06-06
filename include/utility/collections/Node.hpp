#pragma once

#include <Meta.hpp>

namespace DEPOS {

namespace collections {

template <typename V = void, typename P = void, typename N = void> struct Node {
    using Value    = typename Meta::IF<Meta::IsVoid<V>::Result, Meta::Empty, V>::Result;
    using Priority = typename Meta::IF<Meta::IsVoid<P>::Result, Meta::Empty, P>::Result;
    using Next     = typename Meta::IF<Meta::IsVoid<N>::Result, Node *, N *>::Result;

    // auto &next(this auto &self) { return self.next; }
    // void next(this auto &self, auto next) { self.next = next; }

    auto &value(this auto &self) { return self.m_value; }
    void value(this auto &self, Value value) { self.m_value = value; }

    auto criterion(this auto &self) { return self.m_criterion; }

    template <typename T, typename U>
    Node(T &&v, U &&p)
        : m_value(static_cast<V &&>(v)),
          m_criterion(static_cast<P &&>(p)),
          next(nullptr) {}

    template <typename T>
    Node(T &&v)
        : m_value(static_cast<V &&>(v)),
          m_criterion(),
          next(nullptr) {}

    Node()
        : m_value(),
          m_criterion(),
          next(nullptr) {}

  public:
    Value m_value;
    Priority m_criterion;
    Next next;
};

} // namespace collections

} // namespace DEPOS
