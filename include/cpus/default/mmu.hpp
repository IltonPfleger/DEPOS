#pragma once

#include <memory/Memory.hpp>
#include <memory/MemoryMap.hpp>
#include <memory/Segment.hpp>

template <typename Derived = void, size_t P = 0, size_t E = 0>
class DefaultMMU {

  public:
    class PageTable {
      public:
        PageTable() {
            for (auto &e : entries)
                e = 0;
        }
        static constexpr auto Size          = P;
        static constexpr auto EntriesNumber = E;
        alignas(Size) uintptr_t entries[EntriesNumber];
    };

    static void init() {
        // unsigned long KernelTextStart = __kmm.text.start;
        // unsigned long KernelTextEnd   = __kmm.text.end;
        // unsigned long KernelTextSize  = KernelTextEnd - KernelTextStart;

        // unsigned long KernelDataStart = __kmm.data.start;
        // unsigned long KernelDataEnd   = __kmm.data.end;
        // unsigned long KernelDataSize  = KernelDataEnd - KernelDataStart;

        // unsigned long KernelBssStart = __kmm.bss.start;
        // unsigned long KernelBssEnd   = __kmm.bss.end;
        // unsigned long KernelBssSize  = KernelBssEnd - KernelBssStart;

        // unsigned long KernelRODataStart = __kmm.rodata.start;
        // unsigned long KernelRODataEnd   = __kmm.rodata.end;
        // unsigned long KernelRODataSize  = KernelRODataEnd -
        // KernelRODataStart;

        Memory::init();
        using PT = typename Derived::PageTable;
        PT *pt   = new (Memory::kmalloc(sizeof(PT))) PT();

        pt->map(Traits<MemoryMap>::VIRT_ADDR, Traits<MemoryMap>::RAM_BASE,
                Traits<MemoryMap>::RAM_END, PT::Flags::KernelRW);

        pt->map(Traits<MemoryMap>::RAM_BASE, Traits<MemoryMap>::RAM_BASE,
                Traits<Memory>::SIZE, PT::Flags::KernelRW);

        pt->map(Traits<MemoryMap>::UART0, Traits<MemoryMap>::UART0,
                PT::Flags::KernelRW);

        // uintptr_t va = KernelTextStart;
        // pt->map(va, KernelTextStart, KernelTextEnd, PT::Flags::KernelRO);
        // va += KernelTextSize;

        // pt->map(va, KernelRODataStart, KernelRODataEnd, PT::Flags::KernelRO);
        // va += KernelRODataSize;

        // pt->map(va, KernelDataStart, KernelDataEnd, PT::Flags::KernelRW);
        // va += KernelDataSize;

        // pt->map(va, KernelBssStart, KernelBssEnd, PT::Flags::KernelRW);
        // va += KernelBssSize;

        pt->load();
    }
};

template <> class DefaultMMU<void, 0, 0> {
  public:
    class PageTable {
      public:
        enum Flags : uintptr_t {
            UserRO   = 0,
            UserRW   = 0,
            KernelRO = 0,
            KernelRW = 0,
            Default  = UserRW
        };
        static constexpr auto Size = 1;
        auto map(auto &&...) {}
        auto load(auto &&...) {}
    };
    static auto init(auto &&...) {}
};

//
// template <typename T = void>
// class DefaultMMU {
//   public:
//    class PageTable {
//        enum DefaultFlags : uintptr_t { UserRO = 0, UserRW = 0, KernelRO = 0,
//        KernelRW = 0, Default = UserRW };
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
//        static constexpr size_t Size = [] {
//            if constexpr (requires { T::Size; })
//                return T::Size;
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
//            if constexpr (requires { static_cast<T*>(this)->load(args...); })
//            {
//                return static_cast<T*>(this)->load(args...);
//            }
//            return 0;
//        }
//    };
//};
