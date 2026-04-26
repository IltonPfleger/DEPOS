#pragma once

namespace DEPOS {

namespace Meta {

struct Empty {};

template <bool B, typename True, typename False> struct IF {
    using Result = True;
};

template <typename True, typename False> struct IF<false, True, False> {
    using Result = False;
};

template <typename T> struct Void {
    static constexpr bool Result = false;
};

template <> struct Void<void> {
    static constexpr bool Result = true;
};

template <typename T> struct Signed {
    static constexpr bool Result = T(-1) < T(0);
};

template <typename T, typename U> struct Same {
    static constexpr bool Result = false;
};

template <typename T> struct Same<T, T> {
    static constexpr bool Result = true;
};

template <typename T, typename U>
concept SameAs = Same<T, U>::Result;

template <typename T>
concept Integer = requires(T a) {
    a % 10;
    a / 10;
};

template <typename T>
concept Pointer = requires(T t) { []<typename U>(U *) {}(t); };

} // namespace Meta

} // namespace DEPOS

#include <shared/meta/Array.hpp>
#include <shared/meta/Span.hpp>
#include <shared/meta/Tuple.hpp>
#include <shared/meta/TypeList.hpp>
