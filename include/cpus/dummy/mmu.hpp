#pragma once

class DummyMMU {
   public:
    class PageTable {
       public:
        bool map(uintptr_t, uintptr_t, uintptr_t = 0);
    };
    static void init() {}
    static void attach() {}
};
