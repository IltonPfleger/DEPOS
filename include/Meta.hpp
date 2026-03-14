#pragma once

#define offsetof(type, member) ((unsigned long)&(((type *)0)->member))

namespace DEPOS {

namespace Meta {

struct Empty {};

template <bool B, typename True, typename False> struct IF {
    using Result = True;
};

template <typename True, typename False> struct IF<false, True, False> {
    using Result = False;
};

template <typename T, auto Method, typename... Args> struct Caller {
    static auto Result(T *obj, Args... args) { (obj->*Method)(args...); }
};

template <unsigned N, typename T> struct Array {
    constexpr T &operator[](unsigned int row) { return m_data[row]; }
    constexpr const T &operator[](unsigned int row) const { return m_data[row]; }

  private:
    T m_data[N];
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

template <typename... Ts, typename F> void ForEachTypeList(Meta::TypeList<Ts...>, F &&f) {
    (f.template operator()<Ts>(), ...);
}

template <typename T>
concept Integer = requires(T a) {
    a % 10;
    a / 10;
};

template <typename T>
concept Pointer = requires(T t) { []<typename U>(U *) {}(t); };

} // namespace Meta

} // namespace DEPOS
