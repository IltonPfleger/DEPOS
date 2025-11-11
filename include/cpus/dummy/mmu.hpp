#pragma once

class DummyMMU {
   public:
    static constexpr size_t PageSize = 1;
    class PageTable {
       public:
        bool map(uintptr_t, uintptr_t, uintptr_t = 0) { return true; }
    };
    static void set(uintptr_t) {}
    static void init() {}
    static void attach() {}
};
