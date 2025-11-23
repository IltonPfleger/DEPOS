#pragma once

#define offsetof(type, member) ((unsigned long)&(((type *)0)->member))

struct Empty {};

namespace Meta {
template <bool B, typename True, typename False = Empty> struct TypeSelector {
    using Result = True;
};

template <typename True, typename False> struct TypeSelector<false, True, False> {
    using Result = False;
};

//template <typename True> struct TypeSelector<false, True, void> {
//    using Result = Empty;
//};

template <typename T, bool B> struct ConditionalValue {
    T Result;
};

template <typename T> struct ConditionalValue<T, false> {};

template <typename T, auto Method, typename... Args> struct Caller {
    static auto Result(T *obj, Args &&...args) { (obj->*Method)(args...); }
};

template <typename T, typename U> struct SAME {
    static constexpr bool Result = false;
};

template <typename T> struct SAME<T, T> {
    static constexpr bool Result = true;
};

template <typename T> struct VOID {
    static constexpr bool Result = false;
};

template <> struct VOID<void> {
    static constexpr bool Result = true;
};

template <typename T> struct SIGNED {
    static constexpr bool Result = T(-1) < T(0);
};

template <typename T> struct ArrayType {
    using Result = void;
};

template <typename T, long unsigned int N> struct ArrayType<T[N]> {
    using Result = T;
};

template <typename T>
concept INTEGRAL = requires(T a) { a % 2; };

template <typename T>
concept POINTER = requires(T a) { *a; };

template <typename T>
concept IsArray = __is_array(T);

// template <typename T>
// struct REMOVE_POINTER {
//     using Result = T;
// };

// template <typename T>
// struct REMOVE_POINTER<T *> {
//     using Result = T;
// };

// template <typename T>
// struct IS_POINTER {
//     static constexpr bool Result = false;
// };

// template <typename T>
// struct IS_POINTER<T *> {
//     static constexpr bool Result = true;
// };
}; // namespace Meta
