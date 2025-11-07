#pragma once

//#include <ELF.hpp>
//#include <Traits.hpp>
//#include <utils/Console.hpp>
//
//extern "C" const char __KERNEL_END__[];
//
//class Application {
//  public:
//    static uintptr_t start() { return reinterpret_cast<uintptr_t>(const_cast<char *>(__KERNEL_END__)); }
//    static uintptr_t end() {
//        ELF *elf = reinterpret_cast<ELF *>(start());
//        while (elf->valid()) {
//            elf = reinterpret_cast<ELF *>(reinterpret_cast<uintptr_t>(elf) + elf->size());
//        }
//        return reinterpret_cast<uintptr_t>(elf);
//    };
//
//    static void init() {
//        uintptr_t i = start();
//        for (;;) {
//            ELF *elf = reinterpret_cast<ELF *>(i);
//            if (!elf->valid())
//                break;
//
//            // for (ProgramHeader *phdr = phdrs.begin(); phdr != phdrs.end(); phdr++) {
//
//            //Console::out << elf->dyn() << "\n";
//            i += elf->size();
//        }
//    };
//};
