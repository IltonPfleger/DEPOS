template <typename Allocator> class SV39_MMU {
  private:
    static constexpr unsigned long Mode = 8UL << 60;
    static constexpr unsigned long Giga = (1 << 30);

  public:
    class PageTable {
        friend SV39_MMU;

      public:
        PageTable() = default;
        enum Flags {
            V = 1 << 0, // Valid
            R = 1 << 1, // Readable
            W = 1 << 2, // Writable
            X = 1 << 3, // Executable
            U = 1 << 4, // User accessible
            A = 1 << 6, // Accessed
            D = 1 << 7, // Dirty
            UserRO = A | D | R | U | V | X,
            UserRW = A | D | R | U | V | W | X,
            KernelRO = A | D | R | V | X,
            KernelRW = A | D | R | V | W | X,
            Default = UserRW
        };

        void load() const {
            csrw<SupervisorMode::SATP>(Mode |
                                       reinterpret_cast<uintptr_t>(this) >> 12);
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
            if ((size % Giga == 0) && ((va % Giga == 0) && (pa % Giga == 0))) {
                uintptr_t vpn2 = (va >> 30) & 0x1FF;
                this->set(vpn2, pa, flags);
            } else {
                uintptr_t end = pa + size;
                for (; pa < end;) {
                    map(va, pa, flags);
                    va += Size;
                    pa += Size;
                }
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

    // TODO: Don't need to be here
    // class KernelPageTable {
    //  private:
    //    KernelPageTable();
    //    static inline PageTable *instance_s = nullptr;

    //  public:
    //    static void init() {
    //        instance_s = new (Memory::kmalloc(PageTable::Size)) PageTable();
    //        instance_s->map(Traits<MemoryMap>::RAM_BASE,
    //                        Traits<MemoryMap>::RAM_BASE, Traits<Memory>::SIZE,
    //                        PageTable::KernelRW);
    //        instance_s->map(Traits<MemoryMap>::UART, Traits<MemoryMap>::UART,
    //                        PageTable::KernelRW);
    //        instance_s->load();
    //    }
    //    static const PageTable *get() { return instance_s; }
    //};
};
