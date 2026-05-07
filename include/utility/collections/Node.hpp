#pragma once

#include <Meta.hpp>

namespace DEPOS {

namespace collections {

template <typename V = void, typename P = void, typename D = void> struct Node {
    using Value    = typename Meta::IF<Meta::Void<V>::Result, Meta::Empty, V>::Result;
    using Priority = typename Meta::IF<Meta::Void<P>::Result, Meta::Empty, P>::Result;
    using Next     = typename Meta::IF<Meta::Void<D>::Result, Node *, D *>::Result;

    auto &next(this auto &self) { return self._next; }
    void next(this auto &self, auto next) { self._next = next; }

    auto value(this auto &self) { return self._value; }
    auto criterion(this auto &self) { return self._criterion; }

    template <typename T, typename U>
    Node(T &&v, U &&p)
        : _value(static_cast<V &&>(v)),
          _criterion(static_cast<P &&>(p)),
          _next(nullptr) {}

    template <typename T>
    Node(T &&v)
        : _value(static_cast<V &&>(v)),
          _criterion(),
          _next(nullptr) {}

    Node()
        : _value(),
          _criterion(),
          _next(nullptr) {}

  protected:
    Value _value;
    Priority _criterion;
    Next _next;
};

} // namespace collections

} // namespace DEPOS
