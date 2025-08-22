#pragma once

#define OFFSET_OF(type, member) ((unsigned long)&(((type*)0)->member))
#define ASM(i) asm(#i)

struct Meta {
    template <bool B, typename True, typename False>
    struct TypeSelector {
        using Result = True;
    };

    template <typename True, typename False>
    struct TypeSelector<false, True, False> {
        using Result = False;
    };

    static consteval bool StringCompare(const char* a, const char* b) {
        if (!a || !b) return false;
        for (int i = 0;; ++i) {
            if (a[i] != b[i]) return false;
            if (a[i] == '\0') return true;
        }
    }

    // template <bool Cond, typename T = void, typename F = void>
    // struct IF {
    //     using Result = F;
    // };

    // template <typename T, typename F>
    // struct IF<true, T, F> {
    //     using Result = T;
    // };

    template <typename T, typename U>
    struct SAME {
        static constexpr bool Result = false;
    };

    template <typename T>
    struct SAME<T, T> {
        static constexpr bool Result = true;
    };

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
};
