#pragma once

#include <Meta.hpp>

template <size_t P = 0, size_t E = 0>
class DefaultMMU {
   public:
    class PageTable {
       public:
        PageTable() {
            for (auto& e : entries) e = 0;
        }
        static constexpr auto PageSize      = P;
        static constexpr auto EntriesNumber = E;
        alignas(PageSize) uintptr_t entries[EntriesNumber];
    };
};

template <>
class DefaultMMU<0, 0> {
   public:
    class PageTable {
       public:
        enum Flags : uintptr_t { UserRO = 0, UserRW = 0, KernelRO = 0, KernelRW = 0, Default = UserRW };
        static constexpr auto PageSize = 1;
        auto map(auto&&...) {}
        auto load(auto&&...) {}
    };
};

//
// template <typename T = void>
// class DefaultMMU {
//   public:
//    class PageTable {
//        enum DefaultFlags : uintptr_t { UserRO = 0, UserRW = 0, KernelRO = 0, KernelRW = 0, Default = UserRW };
//
//        template <typename V, bool = Meta::VOID<V>::Result>
//        struct HasFlags {
//            using Result = DefaultFlags;
//        };
//
//        template <typename V>
//        struct HasFlags<V, false> {
//            using Result = typename V::Flags;
//        };
//
//       public:
//        using Flags                      = HasFlags<T>::Result;
//        static constexpr size_t PageSize = [] {
//            if constexpr (requires { T::PageSize; })
//                return T::PageSize;
//            else
//                return 1;
//        }();
//
//        auto map(auto&&... args) {
//            if constexpr (requires { static_cast<T*>(this)->map(args...); }) {
//                return static_cast<T*>(this)->map(args...);
//            }
//            return 0;
//        }
//        auto load(auto&&... args) {
//            if constexpr (requires { static_cast<T*>(this)->load(args...); }) {
//                return static_cast<T*>(this)->load(args...);
//            }
//            return 0;
//        }
//    };
//};
