template <typename Allocator> class SV39_MMU {
  private:
    static constexpr unsigned long Mode = 8UL << 60;
    static constexpr unsigned long Giga = (1 << 30);

  public:
    class PageTable {
        friend SV39_MMU;

      public:
        PageTable() {
            for (unsigned int i = 0; i < EntriesNumber; i++) {
                entries[i] = 0;
            }
        };
        enum Flags {
            V = 1 << 0, // Valid
            R = 1 << 1, // Readable
            W = 1 << 2, // Writable
            X = 1 << 3, // Executable
            U = 1 << 4, // User accessible
            A = 1 << 6, // Accessed
            D = 1 << 7, // Dirty
            UserRO = A | D | R | U | V,
            UserRW = A | D | R | U | V | W,
            UserRWX = A | D | R | U | V | W | X,
            KernelRO = A | D | R | V,
            KernelRW = A | D | R | V | W,
            KernelRWX = A | D | R | V | W | X,
            Default = UserRWX
        };

        void load() const {
            csrw<Supervisor::SATP>(Mode | reinterpret_cast<uintptr_t>(this) >> 12);
            CPU::TLB::flush();
        }

        bool map(uintptr_t va, uintptr_t pa, Flags flags) {
            uintptr_t vpn2 = (va >> 30) & 0x1FF;
            uintptr_t vpn1 = (va >> 21) & 0x1FF;
            uintptr_t vpn0 = (va >> 12) & 0x1FF;

            PageTable *l1;
            PageTable *l0;

            if (!entries[vpn2]) {
                l1 = new (Allocator::kmalloc(Size)) PageTable();
                set(vpn2, reinterpret_cast<uintptr_t>(l1), V);
            } else {
                l1 = walk(vpn2);
            }
            if (!l1->entries[vpn1]) {
                l0 = new (Allocator::kmalloc(Size)) PageTable();
                l1->set(vpn1, reinterpret_cast<uintptr_t>(l0), V);
            } else {
                l0 = l1->walk(vpn1);
            }

            return l0->set(vpn0, reinterpret_cast<uintptr_t>(pa), flags);
        }
        bool map(uintptr_t va, Flags flags) { return map(va, va, flags); }

        void map(uintptr_t va, uintptr_t pa, size_t size, Flags flags) {
            while (size >= Giga && (va % Giga == 0) && (pa % Giga == 0)) {
                uintptr_t vpn2 = (va >> 30) & 0x1FF;
                set(vpn2, pa, flags);
                va += Giga;
                pa += Giga;
                size -= Giga;
            }

            while (size > 0) {
                map(va, pa, flags);
                va += Size;
                pa += Size;
                size -= Size;
            }
        }

      private:
        bool set(int vpn, uintptr_t addr, Flags flags) {
            if (entries[vpn])
                return false;
            entries[vpn] = (addr >> 2) | flags;
            return true;
        }

        PageTable *walk(int vpn) {
            uintptr_t pte = entries[vpn];
            uintptr_t addr = (pte >> 10) << 12;
            return reinterpret_cast<PageTable *>(addr);
        }

      private:
        static constexpr auto Size = 4096;
        static constexpr auto EntriesNumber = 512;
        alignas(Size) uintptr_t entries[EntriesNumber];
    };

    static void init() {
        static_assert(Traits<MemoryMap>::PhysicalRamStart % Giga == 0);
        static_assert(Traits<MemoryMap>::VirtualRamStart % Giga == 0);
        static_assert(Traits<MemoryMap>::MMIOStart % Giga == 0);
        if constexpr (Traits<System>::MULTITASK) {
            PageTable &pt = s_base.Result;
            new (&pt) PageTable();
            if (CPU::id() == Traits<::Machine>::BSP) {
                pt.map(Traits<MemoryMap>::VirtualRamStart, Traits<MemoryMap>::PhysicalRamStart, Traits<Memory>::SIZE,
                       PageTable::KernelRWX);
                pt.map(Traits<MemoryMap>::PhysicalRamStart, Traits<MemoryMap>::PhysicalRamStart, Traits<Memory>::SIZE,
                       PageTable::KernelRWX);
                pt.map(Traits<MemoryMap>::MMIOStart, Traits<MemoryMap>::MMIOStart,
                       Traits<MemoryMap>::MMIOEnd - Traits<MemoryMap>::MMIOStart, PageTable::KernelRWX);
            }
            CPU::barrier();
            pt.load();
        }
    }

  private:
    __attribute__((section(".mmu"))) static inline Meta::ConditionalValue<PageTable, Traits<System>::MULTITASK> s_base;
};
