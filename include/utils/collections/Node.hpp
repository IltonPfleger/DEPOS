#pragma once

#include <Meta.hpp>

namespace DEPOS {

namespace collections {

template <typename Value, typename Priority = void, typename Derived = void> struct Node {
    auto &next() { return m_next; }
    const auto &next() const { return m_next; }

    auto &value() { return m_value; }
    const auto &value() const { return m_value; }

    auto &criterion() { return m_criterion; }
    const auto &criterion() const { return m_criterion; }

    template <typename V, typename P>
    Node(V &&v, P &&p)
        : m_value(static_cast<V &&>(v)),
          m_criterion(static_cast<P &&>(p)) {}

    template <typename V>
    Node(V &&v)
        : m_value(static_cast<V &&>(v)) {}

    Node() = default;

    Meta::IF<Meta::Void<Value>::Result, Meta::Empty, Value>::Result m_value;
    Meta::IF<Meta::Void<Priority>::Result, Meta::Empty, Priority>::Result m_criterion;
    Meta::IF<Meta::Void<Derived>::Result, Node *, Derived *>::Result m_next;
};

} // namespace collections

} // namespace DEPOS
