#pragma once

#include <Meta.hpp>
#include <Types.hpp>

class ELF64 {
    struct Header {
        uint8_t ident[16];
        uint16_t type;
        uint16_t machine;
        uint32_t version;
        uint64_t main;
        uint64_t phdroff;
        uint64_t shdroff;
        uint32_t cpu_flags;
        uint16_t hdrsz;
        uint16_t phdrsz;
        uint16_t phdrn;
        uint16_t shdrsz;
        uint16_t shdrn;
        uint16_t _;
    };

    struct ProgramHeader {
        uint32_t type;
        uint32_t flags;
        uint64_t offset;
        uint64_t vaddr;
        uint64_t paddr;
        uint64_t filesz;
        uint64_t memsz;
        uint64_t align;
    };

   public:
    uint64_t entry() const { return header_.main; }

    bool valid() const {
        return header_.ident[0] == 0x7F && header_.ident[1] == 'E' && header_.ident[2] == 'L' &&
               header_.ident[3] == 'F';
    }

    size_t size() const {
        ProgramHeader* phdr = reinterpret_cast<ProgramHeader*>(reinterpret_cast<uintptr_t>(this) + header_.phdroff);
        uint64_t max        = 0;
        for (int i = 0; i < header_.phdrn; i++) {
            uint64_t end = phdr[i].offset + phdr[i].filesz;
            if (end > max) max = end;
        }
        return max;
    }

   private:
    Header header_;
};

typedef Meta::TypeSelector<sizeof(void*) == 8, ELF64, void>::Result ELF;
