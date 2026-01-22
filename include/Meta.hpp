#pragma once

#define offsetof(type, member) ((unsigned long)&(((type *)0)->member))

namespace Meta {
template <bool B, typename True, typename False> struct TypeSelector {
    using Result = True;
};

template <typename True, typename False> struct TypeSelector<false, True, False> {
    using Result = False;
};

static consteval bool StringCompare(const char *a, const char *b) {
    if (!a || !b)
        return false;
    for (int i = 0;; ++i) {
        if (a[i] != b[i])
            return false;
        if (a[i] == '\0')
            return true;
    }
}

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

template <typename T> struct Signed {
    static constexpr bool Result = T(-1) < T(0);
};

template <typename T> struct ArrayType {
    using Result = void;
};

template <typename T, long unsigned int N> struct ArrayType<T[N]> {
    using Result = T;
};

template <typename T>
concept Integer = requires(T a) {
    a % 10;
    a / 10;
};

template <typename T> struct _Pointer {
    static constexpr bool Result = false;
};

template <typename T> struct _Pointer<T *> {
    static constexpr bool Result = true;
};

template <typename T>
concept Pointer = _Pointer<T>::Result;

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
