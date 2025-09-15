#include <Heap.hpp>
#include <Memory.hpp>
#include <cpus/riscv/mmu.hpp>

// void SV39_MMU::map(PageTable* l2, uintptr_t va, uintptr_t pa, Register flags) {
//     uintptr_t vpn2 = (va >> 30) & 0x1FF;
//     uintptr_t vpn1 = (va >> 21) & 0x1FF;
//     uintptr_t vpn0 = (va >> 12) & 0x1FF;
//
//     PageTable* l1;
//     if (!l2->entries[vpn2].value) {
//         l1 = new (Memory::kmalloc()) PageTable();
//         l2->entries[vpn2].next(reinterpret_cast<uintptr_t>(l1), V);
//     } else {
//         l1 = l2->entries[vpn2].next();
//     }
//
//     PageTable* l0;
//     if (!l1->entries[vpn1].value) {
//         l0 = new (Memory::kmalloc()) PageTable();
//         l1->entries[vpn1].next(reinterpret_cast<uintptr_t>(l0), V);
//     } else {
//         l0 = l1->entries[vpn1].next();
//     }
//
//     l0->entries[vpn0].next(pa, flags);
// }

void SV39_MMU::init() {
    //_kernel_page_table = new (Memory::kmalloc()) PageTable();

    // uintptr_t page_size              = Traits::System::PAGE_SIZE;
    // uintptr_t kernel_end_aligned     = (Memory::Map::kernel_end + page_size - 1) & ~(page_size - 1);
    // uintptr_t kernel_size            = kernel_end_aligned - Memory::Map::kernel_start;
    // uintptr_t number_of_kernel_pages = kernel_size / page_size;

    // for (uintptr_t addr = Memory::Map::kernel_start; addr < kernel_end_aligned; addr += page_size) {
    //     uintptr_t va = kernel_virt_base + (addr - Memory::Map::kernel_start);
    //     map(_kernel_page_table, va, addr, R | W | X);
    // }

    // TRACE("[MMU::init]{\n")
    // TRACE("KernelPageTable=(%p,%p)\n", Memory::Map::kernel_start, kernel_end_aligned);
    // TRACE("NumberOfKernelPages=%d\n", number_of_kernel_pages);
    // TRACE("}\n")

    // RISCV::csrw<RISCV::Supervisor::SATP>(SV39 | (reinterpret_cast<uintptr_t>(_kernel_page_table) >> 12));
}
