#pragma once

class SV39_MMU {
    enum {
        V = 1 << 0,  // Valid
        R = 1 << 1,  // Readable
        W = 1 << 2,  // Writable
        X = 1 << 3,  // Executable
        U = 1 << 4,  // User accessible
        A = 1 << 6,  // Accessed
        D = 1 << 7,  // Dirty
    };

   public:
    struct PageTable {
        // struct Entry {
        //     Entry() = default;

        //    void next(uintptr_t addr, Register flags) {
        //        uintptr_t ppn = addr >> 12;
        //        value         = (ppn << 10) | flags;
        //    }

        //    PageTable* next() const { return reinterpret_cast<PageTable*>(value); }

        //    uintptr_t value = 0;
        //};

        // PageTable() = default;
        // alignas(4096) Entry entries[512];
    };

    // wstatic void map(PageTable* root, uintptr_t, uintptr_t, Register);
    static void init();

   private:
    static inline PageTable* _kernel_page_table;
};
