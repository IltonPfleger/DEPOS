#include <Heap.hpp>
#include <IO/Debug.hpp>
#include <Machine.hpp>
#include <Memory.hpp>
#include <cpus/riscv/mmu.hpp>

extern "C" const char __KERNEL_START__[];
extern "C" const char __KERNEL_END__[];

bool SV39_MMU::PageTable::map(uintptr_t va, uintptr_t pa, Flags flags) {
    uintptr_t vpn2 = (va >> 30) & 0x1FF;
    uintptr_t vpn1 = (va >> 21) & 0x1FF;
    uintptr_t vpn0 = (va >> 12) & 0x1FF;

    PageTable* l1;
    PageTable* l0;

    if (!entries[vpn2]) {
        l1 = new (Memory::kmalloc()) PageTable();
        set(vpn2, reinterpret_cast<uintptr_t>(l1), V);
    } else {
        l1 = walk(vpn2);
    }
    if (!l1->entries[vpn1]) {
        l0 = new (Memory::kmalloc()) PageTable();
        l1->set(vpn1, reinterpret_cast<uintptr_t>(l0), V);
    } else {
        l0 = l1->walk(vpn1);
    }

    return l0->set(vpn0, reinterpret_cast<uintptr_t>(pa), flags);
}

SV39_MMU::PageTable* SV39_MMU::base() {
    PageTable* root = new (Memory::kmalloc()) PageTable();

    uintptr_t PAGE_SIZE    = Traits::Memory::Page::SIZE;
    uintptr_t KERNEL_START = reinterpret_cast<uintptr_t>(__KERNEL_START__);
    uintptr_t KERNEL_END   = reinterpret_cast<uintptr_t>(__KERNEL_END__);
    KERNEL_END             = (KERNEL_END + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);

    for (uintptr_t pa = KERNEL_START; pa < KERNEL_END; pa += PAGE_SIZE) root->map(pa, pa);

    root->map(Machine::IO::Addr, Machine::IO::Addr);

    return root;
}

void SV39_MMU::set(uintptr_t root) {
    Machine::CPU::csrw<Machine::CPU::Supervisor::SATP>(MODE | root >> 12);
    asm volatile("sfence.vma zero, zero");
}

// uintptr_t SV39_MMU::attach(uintptr_t addr) {
//     uintptr_t satp  = Machine::CPU::csrr<Machine::CPU::Supervisor::SATP>();
//     PageTable* root = reinterpret_cast<PageTable*>(satp << 12);
//     Flags flags     = static_cast<Flags>(D | A | R | W | X | V);
//     if (map(root, addr, addr, flags)) return addr;
//     return 0;
// };

void SV39_MMU::init() {
    TRACE("[MMU::init]{\n")
    set(reinterpret_cast<uintptr_t>(base()));
    //  const char* teste = "}\n";
    //  TRACE(teste);
    //  TRACE("}\n")
}
