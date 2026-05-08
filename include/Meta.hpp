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

template <unsigned N, typename T> struct Array {
    constexpr T &operator[](unsigned int row) { return m_data[row]; }
    constexpr const T &operator[](unsigned int row) const { return m_data[row]; }

  private:
    T m_data[N];
};

template <typename T, typename U> struct Same {
    static constexpr bool Result = false;
};

template <typename T> struct Same<T, T> {
    static constexpr bool Result = true;
};

template <typename T, typename U>
concept SameAs = Same<T, U>::Result;

template <typename... Tn> struct TypeList {
    static constexpr unsigned int Length = sizeof...(Tn);
};

template <typename List, unsigned int Index> struct GetFromTypeList;

template <typename Head, typename... Tail> struct GetFromTypeList<TypeList<Head, Tail...>, 0> {
    using Result = Head;
};

template <typename Head, typename... Tail, unsigned int Index> struct GetFromTypeList<TypeList<Head, Tail...>, Index> {
    using Result = typename GetFromTypeList<TypeList<Tail...>, Index - 1>::Result;
};

template <typename... Ts, typename Function> void forEach(TypeList<Ts...>, Function f) {
    (f.template operator()<Ts>(), ...);
}

template <typename T>
concept Pointer = requires(T t) { []<typename U>(U *) {}(t); };

template <typename...> struct Tuple;

template <> struct Tuple<> {};

template <typename Head, typename... Tail> struct Tuple<Head, Tail...> {
    Head m_value;
    Tuple<Tail...> m_next;
};

template <typename Function> void forEach(Tuple<> &, Function) {}

template <typename Head, typename... Tail, typename Function> void forEach(Tuple<Head, Tail...> &tuple, Function f) {
    f(tuple.m_value);
    forEach(tuple.m_next, f);
}

/* ------------------------------------------------------------------------- */
/*                                 Types                                     */
/* ------------------------------------------------------------------------- */

template <typename T> struct IsInteger {
    static constexpr bool Result = false;
};
template <> struct IsInteger<char> {
    static constexpr bool Result = true;
};
template <> struct IsInteger<signed char> {
    static constexpr bool Result = true;
};
template <> struct IsInteger<unsigned char> {
    static constexpr bool Result = true;
};
template <> struct IsInteger<short> {
    static constexpr bool Result = true;
};
template <> struct IsInteger<unsigned short> {
    static constexpr bool Result = true;
};
template <> struct IsInteger<int> {
    static constexpr bool Result = true;
};
template <> struct IsInteger<unsigned int> {
    static constexpr bool Result = true;
};
template <> struct IsInteger<long> {
    static constexpr bool Result = true;
};
template <> struct IsInteger<unsigned long> {
    static constexpr bool Result = true;
};
template <> struct IsInteger<long long> {
    static constexpr bool Result = true;
};
template <> struct IsInteger<unsigned long long> {
    static constexpr bool Result = true;
};

template <typename T>
concept Integer = IsInteger<T>::Result;

template <Integer T> struct IsSigned {
    static constexpr bool Result = T(-1) < T(0);
};

template <typename T>
concept Signed = IsSigned<T>::Result;

template <typename T> struct IsVoid {
    static constexpr bool Result = false;
};

template <> struct IsVoid<void> {
    static constexpr bool Result = true;
};

template <typename T>
concept Void = IsVoid<T>::Result;

} // namespace Meta

} // namespace DEPOS
