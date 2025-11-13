#include <cpus/riscv/cpu.hpp>
#include <cpus/riscv/mmu.hpp>
#include <memory/Memory.hpp>

bool SV39_MMU::PageTable::map(uintptr_t va, uintptr_t pa, Flags flags) {
    uintptr_t vpn2 = (va >> 30) & 0x1FF;
    uintptr_t vpn1 = (va >> 21) & 0x1FF;
    uintptr_t vpn0 = (va >> 12) & 0x1FF;

    PageTable* l1;
    PageTable* l0;

    if (!entries[vpn2]) {
        l1 = new (Memory::kmalloc(PageSize)) PageTable();
        set(vpn2, reinterpret_cast<uintptr_t>(l1), V);
    } else {
        l1 = walk(vpn2);
    }
    if (!l1->entries[vpn1]) {
        l0 = new (Memory::kmalloc(PageSize)) PageTable();
        l1->set(vpn1, reinterpret_cast<uintptr_t>(l0), V);
    } else {
        l0 = l1->walk(vpn1);
    }

    return l0->set(vpn0, reinterpret_cast<uintptr_t>(pa), flags);
}

void SV39_MMU::set(uintptr_t root) {
    RISCV::csrw<RISCV::Supervisor::SATP>(Mode | root >> 12);
    RISCV::flush();
}
