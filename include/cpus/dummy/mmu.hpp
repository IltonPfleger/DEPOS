#pragma once

class DummyMMU {
   public:
    class PageTable {
       public:
        bool map(uintptr_t, uintptr_t, uintptr_t = 0) { return true; }
    };
    static void set(uintptr_t) {}
    static void init() {}
    static void attach() {}
};
