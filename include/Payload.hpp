#pragma once

#include <BootInformation.hpp>
#include <Thread.hpp>
#include <Traits.hpp>
#include <libraries/libc/string.h>
#include <memory/Heap.hpp>
#include <memory/Memory.hpp>
#include <utility/ELF/Elf_Ehdr.hpp>
#include <utility/ELF/Elf_Phdr.hpp>

namespace QUARK {

class Payload {
  public:
    static void copy() {
        Elf_Ehdr *header = reinterpret_cast<Elf_Ehdr *>(Image);
        assert(header);
        assert(header->valid());
        load(header);
    }

    static void load(Elf_Ehdr *header) {
        Elf_Phdr *list = reinterpret_cast<Elf_Phdr *>(Image + header->e_phoff);
        for (size_t i = 0; i < header->e_phnum; ++i) {
            Elf_Phdr &phdr = list[i];
            if (phdr.p_type == Elf_Phdr::PT_LOAD) {
                void *destination  = reinterpret_cast<void *>(phdr.p_vaddr);
                const void *source = reinterpret_cast<const void *>(Image + phdr.p_offset);
                if (phdr.p_filesz > 0) {
                    memcpy(destination, source, phdr.p_filesz);
                }

                if (phdr.p_memsz > phdr.p_filesz) {
                    void *bss = reinterpret_cast<void *>(phdr.p_vaddr + phdr.p_filesz);
                    memset(bss, 0, phdr.p_memsz - phdr.p_filesz);
                }
            }
        }
    }

    static void init() {
        Elf_Ehdr *header = reinterpret_cast<Elf_Ehdr *>(Image);
        auto main        = reinterpret_cast<Thread::Return (*)(Thread::Argument)>(header->e_entry);
        new Thread(main, 0, Thread::Criterion::NORMAL);
    };

  private:
    static constexpr size_t Size = Traits<Payload>::Size;
    __attribute__((section(".__payload__"), used)) static inline uint8_t Image[Size];
};

} // namespace QUARK
